/**
 * @file
 */
#ifndef WIN_AUX_H
#define WIN_AUX_H

#include <xcb/xcb.h>

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

#endif
