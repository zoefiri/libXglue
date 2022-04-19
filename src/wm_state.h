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
   int l_width;
   int r_width;
   int t_width;
   int b_width;
} border_t;

typedef struct {
   xcb_ewmh_geometry_t geom;
   char *name;
} desktop_t;

typedef struct {
   xcb_window_t win;
   xcb_window_t decorwin;

   desktop_t *desktop;
   win_geom_t win_geom;
   border_t borders;

   UT_hash_handle hh;
} clientTable;

typedef struct {
   xcb_window_t win;
   desktop_t *desktop;
   xcb_window_t decoration;
} client_t;

typedef struct {
   desktop_t *desktops;
   uint32_t current;
   uint32_t len;
   uint32_t *clients_ordered;
} desktopSet_t;

typedef struct {
   client_t **clients;
   clientTable *client_table;
   uint32_t len;
} clientList_t;

typedef struct {
   xcb_screen_t screen;
   uint32_t screen_i;
} screen_info_t;

typedef struct {
   xcb_ewmh_connection_t *ewmh_c;
   screen_info_t screen;

   xcb_window_t active_win;
   xcb_window_t supporting_win;
   xcb_window_t root;

   xcb_atom_t *ewmh_atoms;

   desktopSet_t desktops;

   clientList_t clients;
   clientList_t virtual_roots;

   border_t default_borders;
   win_geom_t default_window_geom;

   uint8_t showing_desktop;

   char *wm_name;
} ewmh_state_t;

typedef struct {
   ewmh_state_t ewmh_state;
} wm_state_t;  


#endif
