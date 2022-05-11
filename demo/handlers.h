#include <xcb/xcb.h>
#include <libxglue/ev_handle.h>
#include <libxglue/msg_handle.h>

void map_request_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void unmap_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void focus_in_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void focus_out_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void enter_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void leave_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void button_press_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void button_release_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

void motion_handler(xcb_connection_t *c, xcb_generic_event_t *ev, void *wstate);

static ev_handler_t ev_handlers[] = {
   {0x0, NULL},
   {0x1, NULL},
   {XCB_KEY_PRESS, NULL},
   {XCB_KEY_RELEASE, NULL},
   {XCB_BUTTON_PRESS, button_press_handler},
   {XCB_BUTTON_RELEASE, button_release_handler},
   {XCB_MOTION_NOTIFY, motion_handler},
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

/*
 * msg handling 
 */

char *focus_handler(xcb_connection_t *c, paramType_et *params, char *args, void *wm_state);
char *resize_handler(xcb_connection_t *c, paramType_et *params, char *args, void *wm_state);
char *close_handler(xcb_connection_t *c, paramType_et *params, char *args, void *wstate);

static msg_handler_t msg_handlers[] = {
   {
      "focus",
      (paramType_et[]){CHAR_ARG, _END},
      focus_handler
   },
   {
      "resize",
      (paramType_et[]){CHAR_ARG, INT_ARG, _END},
      resize_handler
   },
   {
      "close",
      (paramType_et[]){_END},
      close_handler
   }
};
static int msg_handlers_len = 3;
