#ifndef  WIN_STATE_H
#define  WIN_STATE_H

typedef struct {
   int x;
   int y;
   int w;
   int h;
} win_geom_t;

typedef struct {
   win_geom_t *win_geoms;
} win_state_t;  

#endif
