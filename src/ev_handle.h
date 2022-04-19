/**
 * @file
 * @brief event handling + default handlers
 */
#ifndef  EV_HANDLE_H
#define  EV_HANDLE_H

#include <stdlib.h>
#include <xcb/xcb.h>
#include "wm_state.h"


/**
 * struct {name} - handler structure for XCB events
 */
typedef struct {
   /**
    * @ev handler XCB event response type
    */
   uint32_t ev;
   /**
    * @brief handler function for handler
    *
    * @param c
    * @param ev xcb event returned from xcb_poll_for_event()
    * @param wm_state state data passed to all handlers
    */
   void(*fp)(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate); 
} ev_handler_t;

int handle_ev(xcb_connection_t *c, ev_handler_t *handlers, xcb_generic_event_t *ev, void *wstate);

/**
 * @brief default handler for XCB_MAP_REQUEST
 *
 * @param c 
 * @param ev xcb event returned from xcb_poll_for_event()
 * @param wm_state wm_state data passed to all handlers
 */
void map_request_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

/**
 * @brief default handler for XCB_DESTROY_NOTIFY
 *
 * @param c 
 * @param ev xcb event returned from xcb_poll_for_event()
 * @param wm_state wm_state data passed to all handlers
 */
void unmap_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void focus_in_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void focus_out_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void enter_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void leave_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void button_press_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void button_release_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);
/**
 * @brief event handler array with no handler funcs initialized
 */
static ev_handler_t custom_ev_handlers[] = {
   {0x0, NULL},
   {0x1, NULL},
   {XCB_KEY_PRESS, NULL},
   {XCB_KEY_RELEASE, NULL},
   {XCB_BUTTON_PRESS, NULL},
   {XCB_BUTTON_RELEASE, NULL},
   {XCB_MOTION_NOTIFY, NULL},
   {XCB_ENTER_NOTIFY, NULL},
   {XCB_LEAVE_NOTIFY, NULL},
   {XCB_FOCUS_IN, NULL},
   {XCB_FOCUS_OUT, NULL},
   {XCB_KEYMAP_NOTIFY, NULL},
   {XCB_EXPOSE, NULL},
   {XCB_GRAPHICS_EXPOSURE, NULL},
   {XCB_NO_EXPOSURE, NULL},
   {XCB_VISIBILITY_NOTIFY, NULL},
   {XCB_CREATE_NOTIFY, NULL},
   {XCB_DESTROY_NOTIFY, NULL},
   {XCB_UNMAP_NOTIFY, NULL},
   {XCB_MAP_NOTIFY, NULL},
   {XCB_MAP_REQUEST, NULL},
   {XCB_REPARENT_NOTIFY, NULL},
   {XCB_CONFIGURE_NOTIFY, NULL},
   {XCB_CONFIGURE_REQUEST, NULL},
   {XCB_GRAVITY_NOTIFY, NULL},
   {XCB_RESIZE_REQUEST, NULL},
   {XCB_CIRCULATE_NOTIFY, NULL},
   {XCB_CIRCULATE_REQUEST, NULL},
   {XCB_PROPERTY_NOTIFY, NULL},
   {XCB_SELECTION_CLEAR, NULL},
   {XCB_SELECTION_REQUEST, NULL},
   {XCB_SELECTION_NOTIFY, NULL},
   {XCB_COLORMAP_NOTIFY, NULL},
   {XCB_CLIENT_MESSAGE, NULL},
   {XCB_MAPPING_NOTIFY, NULL},
   {XCB_GE_GENERIC, NULL}
};

/**
 * @brief event handler array with default handlers initialized
 */
static ev_handler_t default_ev_handlers[] = {
   {0x0, NULL},
   {0x1, NULL},
   {XCB_KEY_PRESS, NULL},
   {XCB_KEY_RELEASE, NULL},
   {XCB_BUTTON_PRESS, button_press_handler},
   {XCB_BUTTON_RELEASE, button_release_handler},
   {XCB_MOTION_NOTIFY, NULL},
   {XCB_ENTER_NOTIFY, enter_handler},
   {XCB_LEAVE_NOTIFY, leave_handler},
   {XCB_FOCUS_IN, focus_in_handler},
   {XCB_FOCUS_OUT, focus_out_handler},
   {XCB_KEYMAP_NOTIFY, NULL},
   {XCB_EXPOSE, NULL},
   {XCB_GRAPHICS_EXPOSURE, NULL},
   {XCB_NO_EXPOSURE, NULL},
   {XCB_VISIBILITY_NOTIFY, NULL},
   {XCB_CREATE_NOTIFY, NULL},
   {XCB_DESTROY_NOTIFY, NULL},
   {XCB_UNMAP_NOTIFY, unmap_handler},
   {XCB_MAP_NOTIFY, NULL},
   {XCB_MAP_REQUEST, map_request_handler},
   {XCB_REPARENT_NOTIFY, NULL},
   {XCB_CONFIGURE_NOTIFY, NULL},
   {XCB_CONFIGURE_REQUEST, NULL},
   {XCB_GRAVITY_NOTIFY, NULL},
   {XCB_RESIZE_REQUEST, NULL},
   {XCB_CIRCULATE_NOTIFY, NULL},
   {XCB_CIRCULATE_REQUEST, NULL},
   {XCB_PROPERTY_NOTIFY, NULL},
   {XCB_SELECTION_CLEAR, NULL},
   {XCB_SELECTION_REQUEST, NULL},
   {XCB_SELECTION_NOTIFY, NULL},
   {XCB_COLORMAP_NOTIFY, NULL},
   {XCB_CLIENT_MESSAGE, NULL},
   {XCB_MAPPING_NOTIFY, NULL},
   {XCB_GE_GENERIC, NULL}
};

#endif
