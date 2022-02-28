#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "ev_handle.h"
#include "wm_state.h"

int handle_ev(xcb_connection_t *c, ev_handler_t *handlers, xcb_generic_event_t *ev, void *wm_state) {
   if(handlers[ev->response_type].fp) {
      handlers[ev->response_type].fp(c, ev, wm_state);
   }
   return 0;
}

void map_request_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wm_state) {
   wm_state_t *wstate = (wm_state_t*)wm_state;
   xcb_map_request_event_t *event = (xcb_map_request_event_t*)ev;

   wstate->active_win = event->window;
   uint32_t win_dims[] = {0,0,500,500};

   xcb_map_window(c, event->window);
   xcb_configure_window(c, event->window, XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, win_dims);
}

void destroy_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wm_state) {
   wm_state_t *wstate = (wm_state_t*)wm_state;
   xcb_destroy_notify_event_t *event = (xcb_destroy_notify_event_t*)ev;

   if(event->window == wstate->active_win) {
      wstate->active_win = 0; // this should focus "previous window" etc. in the future but for now it's going to remain like this.
   }

   xcb_kill_client(c, event->window);
}


