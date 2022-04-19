#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <stdlib.h>
#include <uthash.h>
#include <xcb/xproto.h>

#include "ewmh.h"
#include "win_aux.h"
#include "wm_state.h"

xcb_window_t calc_updated_focus(xcb_connection_t *c, ewmh_state_t ewmh_state) { 
   if(ewmh_state.clients.len)
      return ewmh_state.clients.clients[ewmh_state.clients.len-1]->win;
   else
      return 0x0;
}

xcb_window_t getParent(xcb_connection_t *c, xcb_window_t win) {
   xcb_query_tree_cookie_t reply_cookie   =  xcb_query_tree(c, win);
   xcb_query_tree_reply_t  *win_tree      =  xcb_query_tree_reply(c, reply_cookie, NULL);

   xcb_window_t parent = win_tree->parent;
   free(win_tree);
   return parent;
}

xcb_translate_coordinates_reply_t  *calc_absolute_pos(xcb_connection_t *c, xcb_window_t win, xcb_window_t root, int x, int y,  ewmh_state_t *ewmh_state) {
   // on first iteration, target the window decor instead
   xcb_window_t parent = getParent(c, win);
   if (!parent) {
      return NULL;
   }
   if(ewmh_state) {
      xcb_window_t decorwin = get_decorWin(c, win);
      if(decorwin) {
         return calc_absolute_pos(c, decorwin, root, x, y, NULL);
      }
   }
   xcb_translate_coordinates_cookie_t  dst_coord_cookie =  xcb_translate_coordinates(c, win, parent, x, y);
   xcb_translate_coordinates_reply_t   *dst_coord       =  xcb_translate_coordinates_reply(c, dst_coord_cookie, NULL);

   int            x_arg    = dst_coord->dst_x;
   int            y_arg    = dst_coord->dst_y;
   if (parent == root) {
      return dst_coord;
   }
   else {
      free(dst_coord);
      return calc_absolute_pos(c, parent, root, x_arg, y_arg, NULL);
   }
   free(dst_coord);
}

int mvWindow(xcb_connection_t *c, xcb_window_t win, win_geom_t geom, ewmh_state_t *ewmh_state) {
   xcb_window_t decorwin = get_decorWin(c, win);
   if (decorwin) { 
      border_t borders = get_borderDim(c, win);
      if(borders.l_width < 0){
         return 1;
      }
      if(geom.w <= borders.l_width+borders.r_width) geom.w = borders.l_width+borders.r_width;
      if(geom.h <= borders.t_width+borders.b_width) geom.h = borders.t_width+borders.b_width;
      uint32_t geometry[]       = { borders.l_width, borders.t_width,
                                    geom.w-(borders.l_width+borders.r_width), 
                                    geom.h-(borders.t_width+borders.b_width)};
      uint32_t decor_geometry[] = { geom.x, geom.y, geom.w, geom.h };
      printf("basewin: %d %d %d %d\n", geometry[0], geometry[1], geometry[2], geometry[3]);
      printf("decorwin: %d %d %d %d\n", decor_geometry[0], decor_geometry[1], decor_geometry[2], decor_geometry[3]);
      //get and resize decor if decorated
      xcb_configure_window(c, decorwin,
                           XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, decor_geometry);
      xcb_configure_window(c, win, 
                           XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, geometry);
      return 0;
   }
   uint32_t geometry[] = { geom.x, geom.y, geom.w, geom.h };
   xcb_configure_window(c, win, XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, geometry);
   return 0;
}
 
xcb_get_geometry_reply_t *getWindowGeometry(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state) {
   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *geom;
   if (ewmh_state) {
      printf("investigating: 0x%x", ewmh_state->clients);
      xcb_window_t decorwin = get_decorWin(c, win);
      if(decorwin) {
         //retrieve decor geom instead
         cookie = xcb_get_geometry(c, decorwin);
         geom = xcb_get_geometry_reply(c, cookie, NULL);
         return geom;
      }
   }
   cookie = xcb_get_geometry(c, win);
   geom = xcb_get_geometry_reply(c, cookie, NULL);
   return geom;
}

int unmanageWindow(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state) {
   if(ewmh_state->clients.len > 0) {
      clientList_t newList;
      int target_index = -1;
      newList.len = ewmh_state->clients.len - 1;
      newList.clients = calloc(newList.len, sizeof(client_t*));
      for(int i=0; i < ewmh_state->clients.len; i++) {
         if(ewmh_state->clients.clients[i]->win == win) target_index = i;
      }
      if(target_index < 0) { // win not found
         free(newList.clients);
         xcb_unmap_window_checked(c,win);
         return 1;
      }
      else {
         if(ewmh_state->clients.clients[target_index]->decoration) xcb_kill_client(c, ewmh_state->clients.clients[target_index]->decoration);
         for(int i=0; i < ewmh_state->clients.len; i++) {
            if(i < target_index) newList.clients[i]   = ewmh_state->clients.clients[i];
            if(i > target_index) newList.clients[i-1] = ewmh_state->clients.clients[i];
         }
         xcb_flush(c);
      }

      free(ewmh_state->clients.clients);
      ewmh_state->clients = newList;
   }
   else {
      return 1;
   }

   return 0;
}

int manageWindow(xcb_connection_t *c, xcb_window_t win, desktop_t *desktop, ewmh_state_t *ewmh_state) {
   printf("managing window 0x%x %d\n", win);
   // decorate win
   xcb_window_t decor_win = decorWindow(c, win, ewmh_state);
   set_decorWin(c, win, decor_win);
   set_borderDim(c, win, ewmh_state->default_borders);

   clientList_t newList;
   newList.len = ewmh_state->clients.len + 1;
   newList.clients = calloc(newList.len, sizeof(client_t*));

   client_t *newClient = malloc(sizeof(client_t));
   newClient->win = win;
   newClient->desktop = desktop;
   newClient->decoration = decor_win;

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
   
   return 0;
}

xcb_window_t decorWindow(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state) {
   // create border window
	xcb_window_t decor_win = xcb_generate_id(c);
   xcb_gcontext_t decor_gcontext = xcb_generate_id(c);
   uint32_t mask = XCB_CW_BACK_PIXEL;
   uint32_t value[]  = { ewmh_state->screen.screen.white_pixel };
   //
   xcb_get_geometry_reply_t *base_geom = getWindowGeometry(c, win, NULL);
	xcb_void_cookie_t wincookie = xcb_create_window_checked(c, XCB_COPY_FROM_PARENT, decor_win, ewmh_state->root,base_geom->x,base_geom->y,
                                                           base_geom->width,base_geom->height,0,
                                                           XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT, mask, value);
   // set atom to identify decor as border window
   xcb_intern_atom_reply_t *border_atom = xcb_intern_atom_reply(c, xcb_intern_atom(c, 0, strlen("_LIBXGLUE_BORDER"), "_LIBXGLUE_BORDER"), NULL);
   xcb_window_t *atom_data = malloc(sizeof(xcb_window_t));
   *atom_data = win;
   xcb_change_property(c, XCB_PROP_MODE_REPLACE, decor_win, border_atom->atom, XCB_ATOM_WINDOW, 32, 1, atom_data);

   // ensure events from client window are still reported
   const uint32_t select_input_val[] =
   {
      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
         | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW
         | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE
         | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
         | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_POINTER_MOTION 
         | XCB_EVENT_MASK_VISIBILITY_CHANGE
   };
   xcb_change_window_attributes(c, decor_win, XCB_CW_EVENT_MASK, select_input_val);

   // xcb_errors_context_t *ectx;
   // xcb_errors_context_new(c, &ectx);
   // xcb_generic_error_t *err = xcb_request_check(c, wat);
   // if(err) printf("ERR: maj:%s, min:%s, err:%s\n", xcb_errors_get_name_for_major_code(ectx, err->major_code), xcb_errors_get_name_for_minor_code(ectx, err->major_code, err->minor_code), xcb_errors_get_name_for_error(ectx, err->error_code, NULL));
   uint32_t pix_color[] = { ewmh_state->screen.screen.black_pixel }; 
   xcb_create_gc(c, decor_gcontext, ewmh_state->screen.screen.root, XCB_GC_FOREGROUND, pix_color);
   xcb_reparent_window(c, win, decor_win, 0, 0);
   xcb_flush(c);
   return decor_win;
}

xcb_window_t *initClientList(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   xcb_query_tree_cookie_t reply_cookie   =  xcb_query_tree(c, ewmh_state->root);
   xcb_query_tree_reply_t  *root_tree     =  xcb_query_tree_reply(c, reply_cookie, NULL);
   xcb_window_t            *root_children =  xcb_query_tree_children(root_tree);

   client_t **wins = calloc(root_tree->children_len, sizeof(client_t*));
   for(int i=0; i < root_tree->children_len; i++) {
      xcb_get_window_attributes_cookie_t  attributes_cookie = xcb_get_window_attributes(c, root_children[i]);
      xcb_get_window_attributes_reply_t   *attributes_reply = xcb_get_window_attributes_reply(c, attributes_cookie, NULL);
      if(attributes_reply->map_state) {
         client_t *win_ptr = malloc(sizeof(client_t));
         win_ptr->desktop = &ewmh_state->desktops.desktops[ewmh_state->desktops.current];
         win_ptr->win = root_children[i];
         wins[i] = win_ptr;
      }
      free(attributes_reply);
   }

   if(ewmh_state->clients.clients) free(ewmh_state->clients.clients);
   ewmh_state->clients.clients = wins;
   
   free(root_tree);
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

win_geom_t init_win_geom(int x, int y, int w, int h) {
   win_geom_t win_geom;
   win_geom.x = x;
   win_geom.y = y;
   win_geom.w = w;
   win_geom.h = h;
   return win_geom;
}

void add_clientTable(clientTable **client_table, xcb_window_t client, xcb_window_t decorwin, desktop_t *desktop, border_t borders) {
   clientTable *new_client;

   printf("add_table: 0x%x\n", *client_table);
   if(!new_client) {
      new_client = malloc(sizeof *new_client);
      new_client->win = client;
      new_client->decorwin = decorwin;
      new_client->desktop = desktop;
      new_client->borders = borders;
      HASH_ADD_INT(*client_table, win, new_client);
      printf("0x%x client inserted to table\n", client);
   }
}

void del_clientTable(clientTable **client_table, xcb_window_t client) {
   clientTable *target_client;

   printf("del_table: 0x%x\n", *client_table);
   HASH_FIND_INT(*client_table, &client, target_client);
   if(client_table) {
      HASH_DEL(*client_table, target_client);
      free(target_client);
   }
}

clientTable *get_clientTable(clientTable **client_table, xcb_window_t client) {
   clientTable *target_client;

   printf("get_table: 0x%x\n", *client_table);
   HASH_FIND_INT(*client_table, &client, target_client);
   printf("get_table: 0x%x\n", *client_table);
   return target_client;
}

void set_decorWin(xcb_connection_t *c, xcb_window_t win, xcb_window_t decorwin) {
   // set atom for decorwin
   char *atom_str = "_LIBXGLUE_DECORWIN";
   xcb_intern_atom_reply_t *atom = xcb_intern_atom_reply(c, xcb_intern_atom(c, 0, strlen(atom_str), atom_str), NULL);
   xcb_window_t *atom_data = malloc(sizeof(xcb_window_t));
   *atom_data = decorwin;
   xcb_change_property(c, XCB_PROP_MODE_REPLACE, win, atom->atom, XCB_ATOM_WINDOW, 32, 1, atom_data);
}

xcb_window_t get_decorWin(xcb_connection_t *c, xcb_window_t win) {
   // set atom for decorwin
   char *atom_str = "_LIBXGLUE_DECORWIN";
   xcb_intern_atom_reply_t *atom = xcb_intern_atom_reply(c, xcb_intern_atom(c, 0, strlen(atom_str), atom_str), NULL);
   xcb_get_property_reply_t *decorwin_property = xcb_get_property_reply(c, xcb_get_property(c, 0, win, atom->atom,
                                                                                            XCB_ATOM_WINDOW, 0, 1), NULL);
   if(decorwin_property && decorwin_property->value_len) {
      xcb_window_t *decorwin_val = xcb_get_property_value(decorwin_property);
      if(decorwin_val && *decorwin_val) return *decorwin_val;
   }
   return 0x0;
}

void set_borderDim(xcb_connection_t *c, xcb_window_t win, border_t border) {
   // set atom for decorwin
   char *atom_str = "_LIBXGLUE_BORDER_DIM";
   xcb_intern_atom_reply_t *atom = xcb_intern_atom_reply(c, xcb_intern_atom(c, 0, strlen(atom_str), atom_str), NULL);
   xcb_window_t *atom_data = calloc(4, sizeof(uint32_t));
   atom_data[0] = border.l_width;
   atom_data[1] = border.r_width;
   atom_data[2] = border.t_width;
   atom_data[3] = border.b_width;
   xcb_change_property(c, XCB_PROP_MODE_REPLACE, win, atom->atom, XCB_ATOM_INTEGER, 32, 4, atom_data);
}

border_t get_borderDim(xcb_connection_t *c, xcb_window_t win) {
   // set atom for decorwin
   char *atom_str = "_LIBXGLUE_BORDER_DIM";
   xcb_intern_atom_reply_t *atom = xcb_intern_atom_reply(c, xcb_intern_atom(c, 0, strlen(atom_str), atom_str), NULL);
   xcb_get_property_reply_t *border_property = xcb_get_property_reply(c, xcb_get_property(c, 0, win, atom->atom,
                                                                        XCB_ATOM_INTEGER, 0, 4), NULL);
   border_t border;
   if(border_property && border_property->value_len) {
      xcb_window_t *border_val = xcb_get_property_value(border_property);
      border.l_width = border_val[0];
      border.r_width = border_val[1];
      border.t_width = border_val[2];
      border.b_width = border_val[3];
      return border;
   }
   else {
      border.l_width = -1;
      return border;
   }
}

void cleanup(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   for(int i=0; i<ewmh_state->clients.len; i++) {
      xcb_map_window(c, ewmh_state->clients.clients[i]->win);
      xcb_translate_coordinates_reply_t *geom = calc_absolute_pos(c, ewmh_state->clients.clients[i]->win,
                                                                  ewmh_state->screen.screen.root, 0, 0, ewmh_state);
      printf("cleaning 0x%x, %d %d\n", ewmh_state->clients.clients[i]->win, geom->dst_x, geom->dst_y);
      xcb_reparent_window(c, ewmh_state->clients.clients[i]->win, ewmh_state->screen.screen.root, geom->dst_x, geom->dst_y);
   }
   xcb_flush(c);
}
