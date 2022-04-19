#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xproto.h>
#include <xcb/xcb_errors.h>

#include "ev_handle.h"
#include "ewmh.h"
#include "win_aux.h"
#include "wm_state.h"

int handle_ev(xcb_connection_t *c, ev_handler_t *handlers, xcb_generic_event_t *ev, void *wstate) {
   if(handlers[ev->response_type].fp) {
      handlers[ev->response_type].fp(c, ev, wstate);
   }
   return 0;
}

void map_request_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_map_request_event_t *event = (xcb_map_request_event_t*)ev;

   // grab buttonpresses
   xcb_grab_button(c, 1, event->window, XCB_EVENT_MASK_BUTTON_PRESS, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                   XCB_NONE, XCB_NONE, XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);

   // manage the window and put make it EWMH active window + create decor
   manageWindow(c, event->window, NULL, &wm_state->ewmh_state); // desktop needs to be non null, later.

   // map window and move it into place
   xcb_window_t decorwin = get_decorWin(c, event->window);
   mvWindow(c, event->window, wm_state->ewmh_state.default_window_geom, &wm_state->ewmh_state);
   xcb_map_window(c, event->window);
   xcb_map_window(c, decorwin);

   // set active win
   wm_state->ewmh_state.active_win = calc_updated_focus(c, wm_state->ewmh_state);
   ewmh_update_activeWindow(c, &wm_state->ewmh_state);
}

void unmap_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_destroy_notify_event_t *event = (xcb_destroy_notify_event_t*)ev;

   unmanageWindow(c, event->window, &wm_state->ewmh_state);
   wm_state->ewmh_state.active_win = calc_updated_focus(c, wm_state->ewmh_state);
}

void focus_in_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_focus_in_event_t *event = (xcb_focus_in_event_t*)ev;

   printf("focin\n");
}

void focus_out_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_focus_out_event_t *event = (xcb_focus_out_event_t*)ev;

   printf("focout\n");
}

void enter_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_enter_notify_event_t *event = (xcb_enter_notify_event_t*)ev;

   printf("entered\n");
}

void leave_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_leave_notify_event_t *event = (xcb_leave_notify_event_t*)ev;

   printf("left\n");
}

void button_press_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_button_press_event_t *event = (xcb_button_press_event_t*)ev;
   xcb_window_t win = event->event;

   //check if this is border win
   xcb_intern_atom_reply_t *border_atom = xcb_intern_atom_reply(c, xcb_intern_atom(c, 0, strlen("_LIBXGLUE_BORDER"), "_LIBXGLUE_BORDER"), NULL);
   xcb_get_property_reply_t *border_property = xcb_get_property_reply(c, xcb_get_property(c, 0, win, border_atom->atom,
                                                                                          XCB_ATOM_WINDOW, 0, 1), NULL);
   if(border_property->value_len) {
      xcb_window_t *border_atom_val = xcb_get_property_value(border_property);
      if(border_atom_val && *border_atom_val) win = *border_atom_val;
   }

   // xcb_allow_events(c, m, event->time);
   // xcb_grab_button(c, 1, event->event, XCB_EVENT_MASK_BUTTON_PRESS, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
   //                 XCB_NONE, XCB_NONE, XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);
   xcb_set_input_focus(c, XCB_INPUT_FOCUS_NONE, win, event->time);
   wm_state->ewmh_state.active_win = win;

   // stack window on top
   uint32_t values[] = { XCB_STACK_MODE_ABOVE };
   xcb_configure_window (c, event->event, XCB_CONFIG_WINDOW_STACK_MODE, values);

   xcb_flush(c);
   printf("b-down on: 0x%x\n", win);
}

void button_release_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_button_release_event_t *event = (xcb_button_release_event_t*)ev;

   printf("b-up\n");
}
