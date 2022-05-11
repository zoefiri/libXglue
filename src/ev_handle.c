#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xproto.h>
#include <xcb/xcb_errors.h>

#include "ewmh.h"
#include "win_aux.h"
#include "wm_state.h"

int handle_ev(xcb_connection_t *c, ev_handler_t *handlers, xcb_generic_event_t *ev, void *wstate) {
   if(handlers[ev->response_type].fp) {
      handlers[ev->response_type].fp(c, ev, wstate);
   }
   return 0;
}

