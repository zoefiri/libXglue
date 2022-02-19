#include <xcb/xcb.h>
#include <xcb/xproto.h>

void map_request_handler(xcb_connection_t *c, xcb_generic_event_t *ev) {
   xcb_map_request_event_t event = (xcb_map_notify_event_t*)ev;
   printf("window: 0x%x, title: %s\n", event->window, get_wm_name(c, &event->window));
   uint32_t win_dims[] = {0,0,500,500};
   xcb_map_window(c, event->window);
   xcb_configure_window(c, event->window, XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, win_dims);
}

void destroy_handler(xcb_connection_t *c, xcb_generic_event_t *ev) {
   xcb_destroy_notify_event_t event = (xcb_destroy_notify_event_t*)ev;
   printf("ah, 0x%x\n", event->window);
   xcb_kill_client(c, event->window);
   break;
}
