#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_atom.h>
#include <stdlib.h>
#include <uthash.h>
#include <xcb/xproto.h>
#include <string.h>

#include "ewmh.h"
#include "msg_handle.h"
#include "win_aux.h"
#include "wm_state.h"
#include "evloop.h"

msg_handler_t *libxglue_init(xcb_connection_t **c, char *wm_name, msg_handler_t *msg_handlers, int msg_handlers_len, ewmh_state_t *ewmh_state) {
   xcb_screen_t            *screen;
   int                     screen_i;
   xcb_screen_iterator_t   iter;

   *c     =   xcb_connect(NULL, &screen_i);
   screen =   xcb_setup_roots_iterator(xcb_get_setup(*c)).data;

   const uint32_t select_input_val[] =
   {
      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
         | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW
         | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE
         | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
         | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_POINTER_MOTION
   };
   xcb_change_window_attributes(*c,
         screen->root,
         XCB_CW_EVENT_MASK, select_input_val
   );
   if (xcb_poll_for_event(*c) != NULL) {
      perror("another window manager is already running");
      exit(1);
   }
   xcb_flush(*c);


   xcb_aux_sync(*c);
   ewmh_state->root = screen->root;
   ewmh_state->screen.screen = *screen;
   ewmh_state->screen.screen_i = screen_i;
   ewmh_state->wm_name = wm_name;
   ewmh_state->default_borders = init_borders(20, 0, 0, 0, 0);
   ewmh_state->default_window_geom = init_win_geom(0, 0, 500, 500);

   ewmh_init(*c, default_atoms, ewmh_state);

   msg_handler_t *msg_handlers_table = setup_msg_handler_table(msg_handlers, msg_handlers_len);
   return msg_handlers_table;
}

void update_focus(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state) { 
   ewmh_state->active_win = win;
   ewmh_update_activeWindow(c, ewmh_state);
   xcb_set_input_focus(c, XCB_INPUT_FOCUS_NONE, win, XCB_CURRENT_TIME);
}

xcb_window_t getParent(xcb_connection_t *c, xcb_window_t win) {
   xcb_query_tree_cookie_t reply_cookie   =  xcb_query_tree(c, win);
   xcb_query_tree_reply_t  *win_tree      =  xcb_query_tree_reply(c, reply_cookie, NULL);

   if(win_tree) {
      xcb_window_t parent = win_tree->parent;
      free(win_tree);
      return parent;
   }
   else return 0x0;
}

win_geom_t calc_absolute_pos(xcb_connection_t *c, xcb_window_t win, xcb_window_t root, int x, int y,  ewmh_state_t *ewmh_state) {
   xcb_window_t parent = getParent(c, win);
   if (!parent) {
      win_geom_t realpos = {0,0,-1,-1};
      return realpos;
   }
   xcb_translate_coordinates_cookie_t  dst_coord_cookie =  xcb_translate_coordinates(c, win, parent, x, y);
   xcb_translate_coordinates_reply_t   *dst_coord       =  xcb_translate_coordinates_reply(c, dst_coord_cookie, NULL);

   int            x_arg    = dst_coord->dst_x;
   int            y_arg    = dst_coord->dst_y;
   if (parent == root) {
      win_geom_t realpos = {dst_coord->dst_x, dst_coord->dst_y};
      free(dst_coord);
      return realpos;
   }
   else {
      free(dst_coord);
      return calc_absolute_pos(c, parent, root, x_arg, y_arg, NULL);
   }
   free(dst_coord);
}

int mvWindow(xcb_connection_t *c, xcb_window_t win, win_geom_t geom, ewmh_state_t *ewmh_state) {
   uint32_t geometry[] = { geom.x, geom.y, geom.w, geom.h };
   xcb_configure_window(c, win, XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, geometry);
   return 0;
}
 
win_geom_t getWindowGeometry(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state) {
   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *geom;
   cookie = xcb_get_geometry(c, win);
   geom = xcb_get_geometry_reply(c, cookie, NULL);

   win_geom_t ret = {geom->x, geom->y, geom->width, geom->height};
   free(geom);
   return ret;
}

int unmanageWindow(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state) {
   if(ewmh_state->clients.len > 0) {
      int target_index = -1;
      for(int i=0; i < ewmh_state->clients.len; i++) {
         if(ewmh_state->clients.clients[i]->win == win) target_index = i;
      }
      if(target_index < 0) { // win not found
         return 1;
      }
      else {
         clientList_t newList;
         newList.len = ewmh_state->clients.len - 1;
         newList.clients = calloc(newList.len, sizeof(client_t*));
         for(int i=0; i < ewmh_state->clients.len; i++) {
            if(i <  target_index) newList.clients[i] = ewmh_state->clients.clients[i];
            if(i > target_index) newList.clients[i-1] = ewmh_state->clients.clients[i];
         }

         free(ewmh_state->clients.clients);
         ewmh_state->clients = newList;
         ewmh_update_clientList(c, ewmh_state);
      }
   }
   else {
      return 1;
   }
   return 0;
}

int manageWindow(xcb_connection_t *c, xcb_window_t win, desktop_t *desktop, ewmh_state_t *ewmh_state) {
   // update clientList
   clientList_t newList;
   newList.len = ewmh_state->clients.len + 1;
   newList.clients = calloc(newList.len, sizeof(client_t*));

   client_t *newClient = malloc(sizeof(client_t));
   newClient->win = win;
   newClient->desktop = desktop;

   if(ewmh_state->clients.clients) {
      for(int i=0; i < ewmh_state->clients.len; i++)  newList.clients[i] = ewmh_state->clients.clients[i];
      free(ewmh_state->clients.clients);
      newList.clients[newList.len-1] = newClient;
      ewmh_state->clients = newList;
   }
   else {
      ewmh_state->clients            = newList;
      ewmh_state->clients.clients[0] = newClient;
   }
   ewmh_update_clientList(c, ewmh_state);

   // grab buttonpresses
   xcb_grab_button(c, 0, win, XCB_EVENT_MASK_BUTTON_PRESS, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_SYNC,
                   XCB_NONE, XCB_NONE, XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);
   xcb_grab_button(c, 0, win, XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_MOTION, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_SYNC,
                   XCB_NONE, XCB_NONE, XCB_BUTTON_INDEX_1, XCB_MOD_MASK_1);
   
   return 0;
}

border_t init_borders(int wid, int l_wid, int r_wid, int t_wid, int b_wid) {
   border_t border;
   if(wid) {
      border.l_width = wid;
      border.r_width = wid;
      border.t_width = wid;
      border.b_width = wid;
   }
   else {
      border.l_width = l_wid;
      border.r_width = r_wid;
      border.t_width = t_wid;
      border.b_width = b_wid;
   }
   return border;
}

void set_border(xcb_connection_t *c, xcb_window_t win, int width, char *color) {
   int values[] = {width};
   
   unsigned int a,r,g,b;
   if(color) {
      int colorlen = strlen(color);
      if(colorlen == 7) {
         a = 0xff;
         sscanf(color+1, "%02x%02x%02x", &r, &g, &b);
      }
      if(colorlen == 9) {
         sscanf(color+1, "%02x%02x%02x%02x", &a, &r, &g, &b);
      }
   }
   else {
      a = 0x0;
      r = 0x0;
      g = 0x0;
      b = 0x0;
   }

   xcb_configure_window(c, win, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
   values[0] = a << 24 | r << 16 | g << 8 | b;
   xcb_change_window_attributes(c, win, XCB_CW_BORDER_PIXEL, values);
}

border_t get_border(xcb_connection_t *c, xcb_window_t win) {
   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *geom;
   cookie = xcb_get_geometry(c, win);
   geom = xcb_get_geometry_reply(c, cookie, NULL);

   border_t border = init_borders(geom->border_width, 0, 0, 0, 0);
   free(geom);
   return border;
}

win_geom_t init_win_geom(int x, int y, int w, int h) {
   win_geom_t win_geom;
   win_geom.x = x;
   win_geom.y = y;
   win_geom.w = w;
   win_geom.h = h;
   return win_geom;
}

void cleanup(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   for(int i=0; i<ewmh_state->clients.len; i++) {
      xcb_map_window(c, ewmh_state->clients.clients[i]->win);
      win_geom_t geom = calc_absolute_pos(c, ewmh_state->clients.clients[i]->win,
                                          ewmh_state->screen.screen.root, 0, 0, ewmh_state);
      printf("cleaning 0x%x, %d %d\n", ewmh_state->clients.clients[i]->win, geom.x, geom.y);
   }
   xcb_flush(c);
}
