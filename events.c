#include <xcb/xcb.h>
#include <xcb/xproto.h>

void map_request_handler(xcb_connection_t *c, xcb_generic_event_t *ev) {
   xcb_map_request_event_t *event = (xcb_map_request_event_t*)ev;
   uint32_t win_dims[] = {0,0,500,500};
   xcb_map_window(c, event->window);
   xcb_configure_window(c, event->window, XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, win_dims);
}

void destroy_handler(xcb_connection_t *c, xcb_generic_event_t *ev) {
   xcb_destroy_notify_event_t *event = (xcb_destroy_notify_event_t*)ev;
   xcb_kill_client(c, event->window);
}

