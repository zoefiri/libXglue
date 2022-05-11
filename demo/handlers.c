#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <libxglue/wm_state.h>
#include <libxglue/win_aux.h>
#include <libxglue/ewmh.h>
#include <libxglue/msg_handle.h>

void map_request_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_map_request_event_t *event = (xcb_map_request_event_t*)ev;

   // manage the window and put make it EWMH active window
   manageWindow(c, event->window, NULL, &wm_state->ewmh_state); // desktop needs to be non null, later.

   // map window and move it into place
   mvWindow(c, event->window, wm_state->ewmh_state.default_window_geom, &wm_state->ewmh_state);
   xcb_map_window(c, event->window);

   // give it a border
   set_border(c, event->window, 10, "#ff1f3d");

   // set active win
   update_focus(c, event->window, &wm_state->ewmh_state);
}

void unmap_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_destroy_notify_event_t *event = (xcb_destroy_notify_event_t*)ev;

   unmanageWindow(c, event->window, &wm_state->ewmh_state);
   if(wm_state->ewmh_state.clients.len) {
      update_focus(c, wm_state->ewmh_state.clients.clients[wm_state->ewmh_state.clients.len-1]->win, &wm_state->ewmh_state);
      set_border(c, wm_state->ewmh_state.active_win, 10, "#ff1f3d");
   }
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

   // set previous focus border to unfocused border
   set_border(c, wm_state->ewmh_state.active_win, 10, "#ffffff");

   // set focus
   xcb_set_input_focus(c, XCB_INPUT_FOCUS_NONE, win, event->time);
   wm_state->ewmh_state.active_win = win;

   // change border to focused border
   set_border(c, wm_state->ewmh_state.active_win, 10, "#ff1f3d");

   // stack window on top
   uint32_t values[] = { XCB_STACK_MODE_ABOVE };
   xcb_configure_window (c, event->event, XCB_CONFIG_WINDOW_STACK_MODE, values);

   if(!(event->state & XCB_KEY_BUT_MASK_MOD_1)) {
      xcb_allow_events(c, XCB_ALLOW_REPLAY_POINTER, event->time);
   }
   else {
      xcb_allow_events(c, XCB_ALLOW_REPLAY_POINTER | XCB_ALLOW_REPLAY_KEYBOARD, event->time);
   }
   xcb_flush(c);
   printf("b-down on: 0x%x\n", win);
}

void button_release_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_button_release_event_t *event = (xcb_button_release_event_t*)ev;

   printf("b-up\n");
}

void motion_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   xcb_motion_notify_event_t *event = (xcb_motion_notify_event_t*)ev;

   printf("win: 0x%x, ch: 0x%x", event->event, event->child);
   if(event->state & XCB_KEY_BUT_MASK_MOD_1) {
      win_geom_t geom = getWindowGeometry(c, event->event, &wm_state->ewmh_state);
      geom.x = event->root_x - 20;
      geom.y = event->root_y - 20;
      mvWindow(c, event->event, geom, &wm_state->ewmh_state);
   }
}

/*
 *  msg handlers
 */

char *focus_handler(xcb_connection_t *c, paramType_et *params, char *args, void *wstate) {
   args_t parsed = parse_args(params, args);
   return NULL;
}

char *resize_handler(xcb_connection_t *c, paramType_et *params, char *args, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   printf("resiz... 0x%x\n", wm_state->ewmh_state.active_win);
   args_t parsed = parse_args(params, args);

   if(parsed.char_args[0].set && parsed.int_args[0].set && wm_state->ewmh_state.active_win) {
      xcb_window_t target = wm_state->ewmh_state.active_win;
      win_geom_t geom = getWindowGeometry(c, target, &wm_state->ewmh_state);
      switch(parsed.char_args[0].val) {
         case 'l':
            geom.x += -1*parsed.int_args[0].val;
            geom.w += parsed.int_args[0].val;
            break;
         case 'r':
            geom.w += parsed.int_args[0].val;
            break;
         case 'u':
            geom.y += -1*parsed.int_args[0].val;
            geom.h += parsed.int_args[0].val;
            break;
         case 'd':
            geom.h += parsed.int_args[0].val;
            break;
         default:
            return "invalid input";
            break;
      }

      mvWindow(c, target, geom, &wm_state->ewmh_state);
   }
   free_args(parsed);
   return NULL;
}

char *close_handler(xcb_connection_t *c, paramType_et *params, char *args, void *wstate) {
   wm_state_t *wm_state = (wm_state_t*)wstate;
   if(wm_state->ewmh_state.active_win) {
      xcb_unmap_window(c, wm_state->ewmh_state.active_win);
      xcb_kill_client(c, wm_state->ewmh_state.active_win);
      unmanageWindow(c, wm_state->ewmh_state.active_win, &wm_state->ewmh_state);
   }
}
