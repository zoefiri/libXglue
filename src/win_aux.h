/**
 * @file
 */
#ifndef WIN_AUX_H
#define WIN_AUX_H

#include <xcb/xcb.h>

#include "wm_state.h"

/**
 * @brief translates coordinates of a window relative to root
 *
 * @param c 
 * @param win window to calulate position of
 * @param root any ancestor window of win
 * @param x position on win to calculate absolute of
 * @param y position on win to calculate absolute of
 * @return translated coordinates relative to root
 */
xcb_translate_coordinates_reply_t  *calc_absolute_pos(xcb_connection_t *c, xcb_window_t win, xcb_window_t root, int x, int y);

int mvWindow(xcb_connection_t *c, xcb_window_t win, uint32_t geom[4]);
int closeWindow(xcb_connection_t *c, xcb_window_t win, wm_state_t *wm_state);
int manageWindow(xcb_connection_t *c, xcb_window_t win, desktop_t *desktop, wm_state_t *wm_state);
xcb_window_t calc_updated_focus(xcb_connection_t *c, wm_state_t *wm_state);

#endif
