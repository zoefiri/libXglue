#ifndef WIN_AUX_H
#define WIN_AUX_H

#include <xcb/xcb.h>

xcb_translate_coordinates_reply_t  *calc_absolute_pos(xcb_connection_t *c, xcb_window_t *win, xcb_window_t *root, int x, int y);

#endif
