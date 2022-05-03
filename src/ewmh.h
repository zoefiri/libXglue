#ifndef EWMH_H
#define EWMH_H

#include <xcb/xcb.h>
#include "wm_state.h"

typedef enum { _NET_SUPPORTED, _NET_CLIENT_LIST, _NET_NUMBER_OF_DESKTOPS, _NET_DESKTOP_GEOMETRY,
      _NET_DESKTOP_VIEWPORT, _NET_CURRENT_DESKTOP, _NET_DESKTOP_NAMES, _NET_ACTIVE_WINDOW, 
      _NET_WORKAREA, _NET_SUPPORTING_WM_CHECK, _NET_VIRTUAL_ROOTS, _NET_DESKTOP_LAYOUT, 
      _NET_SHOWING_DESKTOP, _NET_CLOSE_WINDOW, _NET_MOVERESIZE_WINDOW, _NET_WM_MOVERESIZE, 
      _NET_RESTACK_WINDOW, _NET_REQUEST_FRAME_EXTENTS, _NET_WM_NAME, _NET_WM_VISIBLE_NAME, 
      _NET_WM_ICON_NAME, _NET_WM_VISIBLE_ICON_NAME, _NET_WM_DESKTOP, _NET_WM_WINDOW_TYPE, 
      _NET_WM_STATE, _NET_WM_ALLOWED_ACTIONS, _NET_WM_STRUT, _NET_WM_STRUT_PARTIAL, _NET_WM_ICON_GEOMETRY,
      _NET_WM_ICON, _NET_WM_PID, _NET_WM_HANDLED_ICONS, _NET_WM_USER_TIME, _NET_FRAME_EXTENTS
} ewmh_atom_et; 

static ewmh_atom_et default_atoms[] = { _NET_SUPPORTED, _NET_CLIENT_LIST, _NET_NUMBER_OF_DESKTOPS, _NET_DESKTOP_GEOMETRY,
                                        _NET_DESKTOP_VIEWPORT, _NET_CURRENT_DESKTOP, _NET_DESKTOP_NAMES, _NET_ACTIVE_WINDOW, 
                                        _NET_WORKAREA, _NET_SUPPORTING_WM_CHECK, _NET_VIRTUAL_ROOTS, 
                                        _NET_SHOWING_DESKTOP, _NET_CLOSE_WINDOW };
                       
static char *atom_names[] = {
   "_NET_SUPPORTED", "_NET_CLIENT_LIST", "_NET_NUMBER_OF_DESKTOPS", "_NET_DESKTOP_GEOMETRY",
   "_NET_DESKTOP_VIEWPORT", "_NET_CURRENT_DESKTOP", "_NET_DESKTOP_NAMES", "_NET_ACTIVE_WINDOW",
   "_NET_WORKAREA", "_NET_SUPPORTING_WM_CHECK", "_NET_VIRTUAL_ROOTS", "_NET_DESKTOP_LAYOUT", 
   "_NET_SHOWING_DESKTOP", "_NET_CLOSE_WINDOW", "_NET_MOVERESIZE_WINDOW", "_NET_WM_MOVERESIZE", 
   "_NET_RESTACK_WINDOW", "_NET_REQUEST_FRAME_EXTENTS", "_NET_WM_NAME", "_NET_WM_VISIBLE_NAME",
   "_NET_WM_ICON_NAME", "_NET_WM_VISIBLE_ICON_NAME", "_NET_WM_DESKTOP", "_NET_WM_WINDOW_TYPE",
   "_NET_WM_STATE", "_NET_WM_ALLOWED_ACTIONS", "_NET_WM_STRUT", "_NET_WM_STRUT_PARTIAL", 
   "_NET_WM_ICON_GEOMETRY","_NET_WM_ICON", "_NET_WM_PID", "_NET_WM_HANDLED_ICONS", 
   "_NET_WM_USER_TIME", "_NET_FRAME_EXTENTS"
};

typedef struct {
   ewmh_atom_et ewmh_atom;
   void(*fp)(xcb_connection_t *c, xcb_client_message_event_t *msg, ewmh_state_t *ewmh_state);
} ewmh_handler_t;

int ewmh_init(xcb_connection_t *c, ewmh_atom_et *atoms, ewmh_state_t *ewmh_state);
int ewmh_initClientList(xcb_connection_t *c, ewmh_state_t *ewmh_state);

int ewmh_update_clientList(xcb_connection_t *c, ewmh_state_t *ewmh_state);
int ewmh_update_numberOfDesktops(xcb_connection_t *c, ewmh_state_t *ewmh_state);
int ewmh_update_desktopGeometry(xcb_connection_t *c, ewmh_state_t *ewmh_state);
int ewmh_update_desktopViewport(xcb_connection_t *c, ewmh_state_t *ewmh_state);
int ewmh_update_currentDesktop(xcb_connection_t *c, ewmh_state_t *ewmh_state);
int ewmh_update_desktopNames(xcb_connection_t *c, ewmh_state_t *ewmh_state);
int ewmh_update_activeWindow(xcb_connection_t *c, ewmh_state_t *ewmh_state);
int ewmh_update_workArea(xcb_connection_t *c, ewmh_state_t *ewmh_state);
int ewmh_update_supportingWmCheck(xcb_connection_t *c, ewmh_state_t *ewmh_state);
int ewmh_update_virtualRoots(xcb_connection_t *c, ewmh_state_t *ewmh_state);
int ewmh_update_showingDesktop(xcb_connection_t *c, ewmh_state_t *ewmh_state);

void ewmh_handle_closeWindow(xcb_connection_t *c, xcb_client_message_event_t *msg, ewmh_state_t *ewmh_state);
void ewmh_handle_moveResizeWindow(xcb_connection_t *c, xcb_client_message_event_t *msg, ewmh_state_t *ewmh_state);
void ewmh_handle_moveResize(xcb_connection_t *c, xcb_client_message_event_t *msg, ewmh_state_t *ewmh_state);
void ewmh_handle_restackWindow(xcb_connection_t *c, xcb_client_message_event_t *msg, ewmh_state_t *ewmh_state);
void ewmh_handle_requestFrameExtents(xcb_connection_t *c, xcb_client_message_event_t *msg, ewmh_state_t *ewmh_state);

static ewmh_handler_t default_ewmh_handlers[] = {
   {_NET_CLOSE_WINDOW, ewmh_handle_closeWindow},
};


#endif 
