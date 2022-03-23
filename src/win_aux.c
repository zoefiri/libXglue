#include <xcb/xcb.h>
#include <stdlib.h>

#include "wm_state.h"

xcb_window_t calc_updated_focus(xcb_connection_t *c, wm_state_t *wm_state) { 
   if(wm_state->ewmh_state.clients.len)
      return wm_state->ewmh_state.clients.clients[wm_state->ewmh_state.clients.len-1]->win;
   else
      return 0x0;
}

xcb_translate_coordinates_reply_t  *calc_absolute_pos(xcb_connection_t *c, xcb_window_t win, xcb_window_t root, int x, int y) {
   printf("calculating root of %x\n", win);
   xcb_query_tree_cookie_t reply_cookie   =  xcb_query_tree(c, win);
   xcb_query_tree_reply_t  *win_tree      =  xcb_query_tree_reply(c, reply_cookie, NULL);

   xcb_translate_coordinates_cookie_t  dst_coord_cookie =  xcb_translate_coordinates(c, win, win_tree->parent, x, y);
   xcb_translate_coordinates_reply_t   *dst_coord       =  xcb_translate_coordinates_reply(c, dst_coord_cookie, NULL);

   int            x_arg    = dst_coord->dst_x;
   int            y_arg    = dst_coord->dst_y;
   xcb_window_t   parent   = win_tree->parent;
   if (win_tree->parent == root) {
      free(win_tree);
      return dst_coord;
   }
   else {
      free(dst_coord);
      free(win_tree);
      return calc_absolute_pos(c, parent, root, x_arg, y_arg);
   }
   free(win_tree);
   free(dst_coord);
}

int mvWindow(xcb_connection_t *c, xcb_window_t win, uint32_t geom[4]) {
   xcb_configure_window(c, win, XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, geom);
   return 0;
}

int closeWindow(xcb_connection_t *c, xcb_window_t win, wm_state_t *wm_state) {
   xcb_kill_client(c, win);
   if(wm_state->ewmh_state.clients.len > 0) {
      clientList_t newList;
      int foundFlag = 0;
      newList.len = wm_state->ewmh_state.clients.len - 1;
      newList.clients = calloc(newList.len, sizeof(client_t*));
      for(int i=0; i < wm_state->ewmh_state.clients.len; i++) {
         if(i < newList.len) {
            if(foundFlag || wm_state->ewmh_state.clients.clients[i]->win != win) {
               printf("i:%d, setting newList:i-1 from ewmhClients:i\n", i);
               newList.clients[i-foundFlag] = wm_state->ewmh_state.clients.clients[i];
            }
            else {
               printf("i:%d, found win 0x%x\n", i, win);
               foundFlag = 1;
               free(wm_state->ewmh_state.clients.clients[i]);
            }
         }
         else if(wm_state->ewmh_state.clients.clients[i]->win == win) {
            printf("i:%d, on last element of ewmhClients, found 0x%x\n", i, win);
            foundFlag = 1;
            free(wm_state->ewmh_state.clients.clients[i]);
         }
      }
      if(!foundFlag) { // win not found
         printf("win 0x%x not found, returning with 1\n", win);
         free(newList.clients);
         return 1;
      }
      printf("writing newList to ewmhClients...\n");
      free(wm_state->ewmh_state.clients.clients);
      wm_state->ewmh_state.clients = newList;
   }
   else {
      return 1;
   }

   return 0;
}

int manageWindow(xcb_connection_t *c, xcb_window_t win, desktop_t *desktop, wm_state_t *wm_state) {
   printf("managing window 0x%x %d\n", win);
   clientList_t newList;
   newList.len = wm_state->ewmh_state.clients.len + 1;
   newList.clients = calloc(newList.len, sizeof(client_t*));

   client_t *newClient = malloc(sizeof(client_t));
   newClient->win = win;
   newClient->desktop = desktop;

   if(wm_state->ewmh_state.clients.clients) {
      for(int i=0; i < wm_state->ewmh_state.clients.len; i++)  newList.clients[i] = wm_state->ewmh_state.clients.clients[i];
      free(wm_state->ewmh_state.clients.clients);
   }
   newList.clients[newList.len-1] = newClient;
   wm_state->ewmh_state.clients = newList;

   return 0;
}
