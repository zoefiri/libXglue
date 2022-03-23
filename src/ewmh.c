#include <string.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "ewmh.h"
#include "wm_state.h"
#include "win_aux.h"


  ////////////////////////////////////////////////////
 // set WM atoms from wm_state                     //
////////////////////////////////////////////////////

int ewmh_init(xcb_connection_t *c, xcb_client_message_event_t *msg, char **atoms, wm_state_t *wm_state) {
   xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(c, wm_state->ewmh_state.ewmh_c);
   xcb_ewmh_init_atoms_replies(wm_state->ewmh_state.ewmh_c, cookie, NULL);

   int i; for(i=0; atoms[i] != NULL; i++);
   wm_state->ewmh_state.ewmh_atoms = calloc(i, sizeof(xcb_atom_t));
   for(int j=0; j < i; j++) {
      xcb_intern_atom_cookie_t atom_cookie = xcb_intern_atom(c, 0, strlen(atoms[j]), atoms[j]);
      xcb_intern_atom_reply_t *atom_reply  = xcb_intern_atom_reply(c, atom_cookie, NULL);
      wm_state->ewmh_state.ewmh_atoms[j]   = atom_reply->atom;
      free(atom_reply);
   }

   wm_state->ewmh_state.active_win = 0x0;

   wm_state->ewmh_state.clients.clients = NULL;
   wm_state->ewmh_state.clients.len = 0;

   wm_state->ewmh_state.desktops.desktops = 0x0;
   wm_state->ewmh_state.desktops.current = 0x0;
   wm_state->ewmh_state.desktops.len = 0;

   wm_state->ewmh_state.showing_desktop = 0;

   wm_state->ewmh_state.supporting_win = ewmh_update_supportingWmCheck(c, wm_state);

   wm_state->ewmh_state.virtual_roots.clients = NULL;
   wm_state->ewmh_state.virtual_roots.len = 0;

   wm_state->ewmh_state.wm_name = "default";

   ewmh_update_activeWindow(c, wm_state);
   ewmh_update_clientList(c, wm_state);

   ewmh_update_currentDesktop(c, wm_state);
   ewmh_update_desktopGeometry(c, wm_state);
   ewmh_update_desktopNames(c, wm_state);
   ewmh_update_desktopViewport(c, wm_state);
   ewmh_update_showingDesktop(c, wm_state);
   ewmh_update_numberOfDesktops(c, wm_state);

   ewmh_update_virtualRoots(c, wm_state);
   ewmh_update_workArea(c, wm_state);

   return 0;
}

int ewmh_update_clientList(xcb_connection_t *c, wm_state_t *wm_state) {
   xcb_window_t *list = calloc(wm_state->ewmh_state.clients.len, sizeof(client_t));
   for(int i=0; i < wm_state->ewmh_state.clients.len; i++) {
      list[i] = (wm_state->ewmh_state.clients.clients[i])->win;
   }

   xcb_void_cookie_t cookie = xcb_ewmh_set_client_list(wm_state->ewmh_state.ewmh_c, wm_state->screen.screen_i,
                                                       wm_state->ewmh_state.clients.len, list);
   xcb_request_check(c, cookie);
   return 0;
}

int ewmh_update_numberOfDesktops(xcb_connection_t *c, wm_state_t *wm_state) {
   xcb_void_cookie_t cookie = xcb_ewmh_set_number_of_desktops(wm_state->ewmh_state.ewmh_c, wm_state->screen.screen_i,
                                                              wm_state->ewmh_state.desktops.len);
   xcb_request_check(c, cookie);
   return 0;
}

int ewmh_update_desktopGeometry(xcb_connection_t *c, wm_state_t *wm_state) {
   xcb_void_cookie_t cookie = xcb_ewmh_set_desktop_geometry(wm_state->ewmh_state.ewmh_c, wm_state->screen.screen_i,
                                                            wm_state->screen.screen.width_in_pixels, wm_state->screen.screen.height_in_pixels);
   xcb_request_check(c, cookie);
   return 0;
}

int ewmh_update_desktopViewport(xcb_connection_t *c, wm_state_t *wm_state) {
   xcb_ewmh_coordinates_t *viewports = calloc(wm_state->ewmh_state.desktops.len, sizeof(xcb_ewmh_coordinates_t));
   for(int i=0; i < wm_state->ewmh_state.desktops.len; i++) {
      viewports[i].x = 0;
      viewports[i].y = 0;
   }
   xcb_void_cookie_t cookie = xcb_ewmh_set_desktop_viewport(wm_state->ewmh_state.ewmh_c, wm_state->screen.screen_i,
                                                            wm_state->ewmh_state.desktops.len, viewports);
   xcb_request_check(c, cookie);
   free(viewports);
   return 0;
}

int ewmh_update_currentDesktop(xcb_connection_t *c, wm_state_t *wm_state) {
   xcb_void_cookie_t cookie = xcb_ewmh_set_current_desktop(wm_state->ewmh_state.ewmh_c, wm_state->screen.screen_i,
                                                           wm_state->ewmh_state.desktops.current);
   xcb_request_check(c, cookie);
   return 0;
}

int ewmh_update_desktopNames(xcb_connection_t *c, wm_state_t *wm_state) {
   int len = 0;
   for(int i=0; i < wm_state->ewmh_state.desktops.len; i++) {
      len += strlen(wm_state->ewmh_state.desktops.desktops[i].name) + 1;
   }
   char *names = calloc(len, sizeof(char));
   int j = 0, k = 0;
   for(int i=0; i < len; i++) {
      if(wm_state->ewmh_state.desktops.desktops[j].name[k]) {
         names[i] = wm_state->ewmh_state.desktops.desktops[j].name[k];
         k++;
      }
      else {
         names[i] = '\0';
         k = 0;
         j++;
      }
   }
   xcb_void_cookie_t cookie = xcb_ewmh_set_desktop_names(wm_state->ewmh_state.ewmh_c, wm_state->screen.screen_i,
                                                         wm_state->ewmh_state.desktops.len, names);
   xcb_request_check(c, cookie);
   free(names);
   return 0;
}

int ewmh_update_activeWindow(xcb_connection_t *c, wm_state_t *wm_state) {
   xcb_void_cookie_t cookie = xcb_ewmh_set_current_desktop(wm_state->ewmh_state.ewmh_c, wm_state->screen.screen_i,
                                                           wm_state->ewmh_state.active_win);
   xcb_request_check(c, cookie);
   return 0;
}

int ewmh_update_workArea(xcb_connection_t *c, wm_state_t *wm_state) {
   xcb_ewmh_geometry_t *workareas = calloc(wm_state->ewmh_state.desktops.len, sizeof(xcb_ewmh_coordinates_t));
   for(int i=0; i < wm_state->ewmh_state.desktops.len; i++) {
      workareas[i] = wm_state->ewmh_state.desktops.desktops[i].geom;
   }
   xcb_void_cookie_t cookie = xcb_ewmh_set_workarea(wm_state->ewmh_state.ewmh_c, wm_state->screen.screen_i,
                                                    wm_state->ewmh_state.desktops.len, workareas);
   xcb_request_check(c, cookie);
   free(workareas);
   return 0;
}

int ewmh_update_supportingWmCheck(xcb_connection_t *c, wm_state_t *wm_state) {
	wm_state->ewmh_state.supporting_win = xcb_generate_id(c);
	xcb_create_window(c, XCB_COPY_FROM_PARENT, wm_state->ewmh_state.supporting_win, wm_state->root,
                     -1, -1, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_ONLY, XCB_COPY_FROM_PARENT, XCB_NONE, NULL);
   xcb_intern_atom_cookie_t atom_cookie   = xcb_intern_atom(c, 0, strlen("_NET_WM_NAME"), "_NET_WM_NAME");
   xcb_intern_atom_reply_t *atom_reply    = xcb_intern_atom_reply(c, atom_cookie, NULL);
   xcb_change_property(c, XCB_PROP_MODE_REPLACE, wm_state->root, atom_reply->atom, XCB_ATOM_STRING, 32,
                       strlen(wm_state->ewmh_state.wm_name), wm_state->ewmh_state.wm_name);

   xcb_void_cookie_t cookie = xcb_ewmh_set_supporting_wm_check(wm_state->ewmh_state.ewmh_c, wm_state->root, wm_state->ewmh_state.supporting_win);
   xcb_request_check(c, cookie);
   return 0;
}

int ewmh_update_virtualRoots(xcb_connection_t *c, wm_state_t *wm_state) {
   xcb_window_t *list = calloc(wm_state->ewmh_state.clients.len, sizeof(client_t));
   for(int i=0; i < wm_state->ewmh_state.clients.len; i++) {
      list[i] = (wm_state->ewmh_state.clients.clients[i])->win;
   }

   xcb_void_cookie_t cookie = xcb_ewmh_set_virtual_roots(wm_state->ewmh_state.ewmh_c, wm_state->screen.screen_i,
                                                         wm_state->ewmh_state.virtual_roots.len, list);
   xcb_request_check(c, cookie);
   return 0;
}

int ewmh_update_showingDesktop(xcb_connection_t *c, wm_state_t *wm_state) {
   wm_state->ewmh_state.showing_desktop = !wm_state->ewmh_state.showing_desktop;
   xcb_void_cookie_t cookie = xcb_ewmh_set_showing_desktop(wm_state->ewmh_state.ewmh_c, wm_state->screen.screen_i,
                                                           wm_state->ewmh_state.showing_desktop);
   xcb_request_check(c, cookie);
   return 0;
}



  ////////////////////////////////////////////////////
 // Handle ewmh client messages                    //
////////////////////////////////////////////////////
void ewmh_handle_closeWindow(xcb_connection_t *c, xcb_client_message_event_t *msg, wm_state_t *wm_state) {
   closeWindow(c, msg->window, wm_state);
}

void ewmh_handle_moveResizeWindow(xcb_connection_t *c, xcb_client_message_event_t *msg, wm_state_t *wm_state){
}

void ewmh_handle_moveResize(xcb_connection_t *c, xcb_client_message_event_t *msg, wm_state_t *wm_state){
}

void ewmh_handle_restackWindow(xcb_connection_t *c, xcb_client_message_event_t *msg, wm_state_t *wm_state){
}

void ewmh_handle_requestFrameExtents(xcb_connection_t *c, xcb_client_message_event_t *msg, wm_state_t *wm_state){
}
