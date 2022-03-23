#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "ev_handle.h"
#include "win_aux.h"
#include "wm_state.h"

int handle_ev(xcb_connection_t *c, ev_handler_t *handlers, xcb_generic_event_t *ev, wm_state_t *wm_state) {
   if(handlers[ev->response_type].fp) {
      handlers[ev->response_type].fp(c, ev, wm_state);
   }
   return 0;
}

void map_request_handler(xcb_connection_t *c, xcb_generic_event_t *ev, wm_state_t *wm_state) {
   xcb_map_request_event_t *event = (xcb_map_request_event_t*)ev;

   uint32_t win_dims[] = {0,0,500,500};

   xcb_map_window(c, event->window);
   mvWindow(c, event->window, win_dims);
   manageWindow(c, event->window, NULL, wm_state); // desktop needs to be non null, later.
   wm_state->ewmh_state.active_win = calc_updated_focus(c, wm_state);
   printf("0x%x opened: focus handed to 0x%x\n", event->window, wm_state->ewmh_state.active_win);
   printf("clients len: %d\n\n", wm_state->ewmh_state.clients.len);
}

void destroy_handler(xcb_connection_t *c, xcb_generic_event_t *ev, wm_state_t *wm_state) {
   xcb_destroy_notify_event_t *event = (xcb_destroy_notify_event_t*)ev;

   printf("clients len CHK: %d\n", wm_state->ewmh_state.clients.len);
   closeWindow(c, event->window, wm_state);
   wm_state->ewmh_state.active_win = calc_updated_focus(c, wm_state);
   printf("0x%x closed: focus handed to 0x%x\n", event->window, wm_state->ewmh_state.active_win);
   printf("clients len: %d\n\n", wm_state->ewmh_state.clients.len);
}


