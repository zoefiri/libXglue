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

msg_handler_t *libxglue_init(xcb_connection_t **c, char *wm_name, msg_handler_t *msg_handlers, int msg_handlers_len, ewmh_state_t *ewmh_state);

int mvWindow(xcb_connection_t *c, xcb_window_t win, win_geom_t geom, ewmh_state_t *ewmh_state);
int unmanageWindow(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state);
int manageWindow(xcb_connection_t *c, xcb_window_t win, desktop_t *desktop, ewmh_state_t *ewmh_state);
xcb_window_t getParent(xcb_connection_t *c, xcb_window_t win);
void update_focus(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state);
border_t init_borders(int wid, int l_wid, int r_wid, int t_wid, int b_wid);
win_geom_t init_win_geom(int x, int y, int w, int h);
win_geom_t getWindowGeometry(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state);
xcb_window_t decorWindow(xcb_connection_t *c, xcb_window_t win, ewmh_state_t *ewmh_state);

void add_clientTable(clientTable **client_table, xcb_window_t client, xcb_window_t decorwin, desktop_t *desktop, border_t borders);
void del_clientTable(clientTable **client_table, xcb_window_t client);
clientTable *get_clientTable(clientTable **client_table, xcb_window_t client);

void set_decorWin(xcb_connection_t *c, xcb_window_t win, xcb_window_t decorwin);
xcb_window_t get_decorWin(xcb_connection_t *c, xcb_window_t win);
border_t get_borderDim(xcb_connection_t *c, xcb_window_t win);
void set_border(xcb_connection_t *c, xcb_window_t win, int width, char *color);
border_t get_border(xcb_connection_t *c, xcb_window_t win);

void cleanup(xcb_connection_t *c, ewmh_state_t *ewmh_state);

#endif
