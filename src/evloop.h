/**
 * @file
 * @brief event loop and its helper functions
 */

#define EVLOOP_H

#include <xcb/xcb.h>
#include "msg_handle.h"
#include "ev_handle.h"

/**
 * @brief enters event loop to handle messages specified in msg_handlers and X events
 *
 * @param c x connection
 * @param ev_handlers statically defined in an array of ev_handler_t 
 * @param {name} reference to hash table of msg_handler_t generated from array of msg_handler_t by setup_msg_handler_table()
 * @param wm_state arbitrary data to be passed to handlers
 * @return evloop exit status
 */
int ev_loop(xcb_connection_t *c, ev_handler_t *ev_handlers, msg_handler_t **msg_handlers, void *wm_state);

#endif // !evloop
