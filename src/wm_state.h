#ifndef  WM_STATE_H
#define  WM_STATE_H

typedef struct {
   int x;
   int y;
   int w;
   int h;
} win_geom_t;

typedef struct {
   win_geom_t *win_geoms;
} wm_state_t;  

#endif
