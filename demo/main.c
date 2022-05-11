#include <xcb/xcb.h>
#include <libxglue/evloop.h>
#include <libxglue/msg_handle.h>
#include <libxglue/ev_handle.h>
#include <libxglue/win_aux.h>
#include <libxglue/wm_state.h>
#include <libxglue/ewmh.h>

#include "handlers.h"

int main() {
   xcb_connection_t        *c;
   wm_state_t wm_state;

   msg_handler_t *msg_handlers_table = libxglue_init(&c, "testwm", msg_handlers, msg_handlers_len, &wm_state.ewmh_state);
   ev_loop(c, ev_handlers, &msg_handlers_table, &wm_state.ewmh_state, &wm_state);
   return 0;
}
