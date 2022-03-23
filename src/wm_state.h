/**
 * @file
*/
#ifndef  WM_STATE_H
#define  WM_STATE_H

#include <uthash.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

typedef struct {
   int x;
   int y;
   int w;
   int h;
} win_geom_t;

typedef struct {
   xcb_ewmh_geometry_t geom;
   char *name;
} desktop_t;

typedef struct {
   xcb_window_t win;
   desktop_t *desktop;
} client_t;

typedef struct {
   desktop_t *desktops;
   uint32_t current;
   uint32_t len;
} desktopSet_t;

typedef struct {
   client_t **clients;
   uint32_t len;
} clientList_t;

typedef struct {
   xcb_screen_t screen;
   uint32_t screen_i;
} screen_info_t;

typedef struct {
   xcb_ewmh_connection_t *ewmh_c;
   xcb_atom_t *ewmh_atoms;
   clientList_t clients;
   desktopSet_t desktops;
   xcb_window_t active_win;
   xcb_window_t supporting_win;
   clientList_t virtual_roots;
   uint8_t showing_desktop;
   char *wm_name;
} ewmh_state_t;

typedef struct {
   ewmh_state_t ewmh_state;
   xcb_window_t root;
   screen_info_t screen;
} wm_state_t;  

#endif
