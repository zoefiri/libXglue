/**
 * @file
 */
#ifndef WIN_AUX_H
#define WIN_AUX_H

#include <xcb/xcb.h>

#include "wm_state.h"
#include "msg_handle.h"
#include "ev_handle.h"

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
win_geom_t calc_absolute_pos(xcb_connection_t *c, xcb_window_t win, xcb_window_t root, int x, int y,  ewmh_state_t *ewmh_state);

/**
 * @brief initializes libxglue (xcb boilerplate and sets up msg handlers hashtable)
 *
 * @param c 
 * @param wm_name name of your window manager
 * @param msg_handlers an array of message handlers for your WM
 * @param msg_handlers_len length of message_handlers
 * @param ewmh_state pointer to ewmh_state_t for storing EWMH state
 * @return hashtable keyed by msg handler names generated from msg_handlers
 */
msg_handler_t *libxglue_init(xcb_connection_t **c, char *wm_name, msg_handler_t *msg_handlers, int msg_handlers_len, ewmh_state_t *ewmh_state);

/**
 * @brief moves the window to the specified geometry
 *
 * @param c 
 * @param win the window to operate on
 * @param geom the new geometry of the window
 * @param ewmh_state 
 */
int mvWindow(xcb_connection_t *c, xcb_window_t win, win_geom_t geom, ewmh_state_t *ewmh_state);

/**
 * @brief moves the window to the specified geometry
 *
 * @param c 
 * @param win the window to operate on
 * @param geom the new geometry of the window
 * @param ewmh_state 
 */
int unmanageWindow(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state);

/**
 * @brief manages the window by putting it into the client list
 *
 * @param c 
 * @param win the window to operate on
 * @param desktop the desktop of the new window to manage
 * @param ewmh_state 
 */
int manageWindow(xcb_connection_t *c, xcb_window_t win, desktop_t *desktop, ewmh_state_t *ewmh_state);

/**
 * @brief gets a window's parent
 *
 * @param c 
 * @param win the window to operate on
 * @return the parent window of win
 */
xcb_window_t getParent(xcb_connection_t *c, xcb_window_t win);

/**
 * @brief updates EWMH active window and sets the X input focus to it
 *
 * @param c 
 * @param win the window to operate on
 * @param ewmh_state 
 */
void update_focus(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state);

/**
 * @brief creates border_t 
 *
 * @param wid if nonzero, sets all sides of the border to wid.
 * @param l_wid left width of border
 * @param r_wid right width of border
 * @param t_wid top width of border
 * @param b_wid bottom width of border
 * @return border_t of specified geometry
 */
border_t init_borders(int wid, int l_wid, int r_wid, int t_wid, int b_wid);

/**
 * @brief creates a win_geom_t
 *
 * @param x upper x coord of window
 * @param y upper y coord of window
 * @param w width of window
 * @param h height of window
 * @return win_geom_t of specified geometry
 */
win_geom_t init_win_geom(int x, int y, int w, int h);

/**
 * @brief gets the geometry of a window
 *
 * @param c 
 * @param win the window to get geometry of
 * @param ewmh_state 
 * @return win_geom_t of window's geometry
 */
win_geom_t getWindowGeometry(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state);

/**
 * @brief gets the border width of a window
 *
 * @param c 
 * @param win the window to get border dimensions of
 * @return border_t representing the dimensions of the window border
 */
border_t get_borderDim(xcb_connection_t *c, xcb_window_t win);

/**
 * @brief sets the border appearance of a window
 *
 * @param c 
 * @param win the window to get geometry of
 * @param width width of window's new border 
 * @param color string representing border's new color, formatted #rrggbb or #aarrggbb
 */
void set_border(xcb_connection_t *c, xcb_window_t win, int width, char *color);

/**
 * @brief gets the border geometry of a window
 *
 * @param c 
 * @param win the window to get border geometry of
 * @return border_t of window's border geometry
 */
border_t get_border(xcb_connection_t *c, xcb_window_t win);

void cleanup(xcb_connection_t *c, ewmh_state_t *ewmh_state);

#endif
