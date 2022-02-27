#ifndef  EV_HANDLE_H
#define  EV_HANDLE_H

#include <stdlib.h>
#include <xcb/xcb.h>


/**
 * contains an xcb event response type and its appropriate handler function pointer
 */
typedef struct {
   uint32_t ev; ///< event response type identifying this handler
   /**
    * takes event and extra data (active keypresses etc.) and handles ev
    * 
    */
   void(*fp)(xcb_connection_t *c, xcb_generic_event_t *ev, void *wm_state); 
} ev_handler_t;

int handle_ev(xcb_connection_t *c, ev_handler_t *handlers, xcb_generic_event_t *ev, void *wm_state);

void map_request_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wm_state);
void destroy_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wm_state);

/**
 * array of handler_t, indexed by corresponding xcb event response type
 * custom variant: all function pointers are set to NULL for user convenience
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
 * array of handler_t, indexed by corresponding xcb event response type
 * default variant: default handlers are provided for all events
 */
static ev_handler_t default_ev_handlers[] = {
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
   {XCB_DESTROY_NOTIFY, destroy_handler},
   {XCB_UNMAP_NOTIFY, NULL},
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
