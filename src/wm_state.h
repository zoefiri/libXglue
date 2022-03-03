/**
 * @file
*/
#ifndef  WM_STATE_H
#define  WM_STATE_H

#include <xcb/xcb.h>

typedef struct {
   int x;
   int y;
   int w;
   int h;
} win_geom_t;

typedef struct {
   win_geom_t *win_geoms;
   xcb_window_t active_win;
   xcb_window_t root;
} wm_state_t;  

#endif
