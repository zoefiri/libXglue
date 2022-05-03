#include <string.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "ewmh.h"
#include "wm_state.h"
#include "win_aux.h"

  ////////////////////////////////////////////////////
 // set WM atoms from ewwh_state                   //
////////////////////////////////////////////////////

int ewmh_init(xcb_connection_t *c, ewmh_atom_et *atoms, ewmh_state_t *ewmh_state) {
   xcb_ewmh_connection_t ewmh;
   xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(c, &ewmh);
   xcb_ewmh_init_atoms_replies(&ewmh, cookie, NULL);
   ewmh_state->ewmh_c = malloc(sizeof(xcb_ewmh_connection_t));
   *ewmh_state->ewmh_c = ewmh;

   int i; for(i=0; atoms[i] != NULL; i++);
   ewmh_state->ewmh_atoms = calloc(i, sizeof(xcb_atom_t));
   for(int j=0; j < i; j++) {
      xcb_intern_atom_cookie_t atom_cookie = xcb_intern_atom(c, 0, strlen(atoms[j]), atoms[j]);
      xcb_intern_atom_reply_t *atom_reply  = xcb_intern_atom_reply(c, atom_cookie, NULL);
      ewmh_state->ewmh_atoms[j]   = atom_reply->atom;
      free(atom_reply);
   }

   ewmh_initClientList(c, ewmh_state);

   if(ewmh_state->clients.clients && ewmh_state->clients.len) {
      ewmh_state->active_win = ewmh_state->clients.clients[ewmh_state->clients.len-1]->win;
   }
   else ewmh_state->active_win = 0x0;

   ewmh_state->desktops.desktops = 0x0;
   ewmh_state->desktops.current = 0x0;
   ewmh_state->desktops.len = 0;

   ewmh_state->showing_desktop = 0;

   ewmh_state->supporting_win = ewmh_update_supportingWmCheck(c, ewmh_state);

   ewmh_state->virtual_roots.clients = NULL;
   ewmh_state->virtual_roots.len = 0;

   ewmh_state->wm_name = "default";

   ewmh_update_activeWindow(c, ewmh_state);
   ewmh_update_clientList(c, ewmh_state);

   ewmh_update_currentDesktop(c, ewmh_state);
   ewmh_update_desktopGeometry(c, ewmh_state);
   ewmh_update_desktopNames(c, ewmh_state);
   ewmh_update_desktopViewport(c, ewmh_state);
   ewmh_update_showingDesktop(c, ewmh_state);
   ewmh_update_numberOfDesktops(c, ewmh_state);

   ewmh_update_virtualRoots(c, ewmh_state);
   ewmh_update_workArea(c, ewmh_state);

   xcb_set_input_focus(c, XCB_INPUT_FOCUS_NONE, ewmh_state->supporting_win, XCB_CURRENT_TIME);

   return 0;
}

int ewmh_initClientList(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   xcb_query_tree_cookie_t reply_cookie   =  xcb_query_tree(c, ewmh_state->root);
   xcb_query_tree_reply_t  *root_tree     =  xcb_query_tree_reply(c, reply_cookie, NULL);
   xcb_window_t            *root_children =  xcb_query_tree_children(root_tree);
   xcb_get_geometry_reply_t          *geom;
   xcb_translate_coordinates_reply_t *pos; 
   win_geom_t                        newpos;
   ewmh_state->clients.clients = NULL;
   ewmh_state->clients.len = 0;

   for(int i=0; i < root_tree->children_len; i++) {
      xcb_get_window_attributes_reply_t *attributes = xcb_get_window_attributes_reply(c, xcb_get_window_attributes(c, root_children[i]), NULL);
      if(attributes->map_state) {
         printf("mng: 0x%x\n", root_children[i]);
         manageWindow(c, root_children[i], NULL, ewmh_state); // desktop needs to be non null, later.
         xcb_flush(c);
      }
      free(attributes);
   }
}

int ewmh_update_clientList(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   xcb_window_t *list = calloc(ewmh_state->clients.len, sizeof(client_t));
   for(int i=0; i < ewmh_state->clients.len; i++) {
      list[i] = (ewmh_state->clients.clients[i])->win;
   }
   printf("clilist: ");
   for(int i=0; i < ewmh_state->clients.len; i++) {
      printf("0x%x, ", list[i]);
   }
   printf("\n");

   xcb_void_cookie_t cookie;
   cookie = xcb_ewmh_set_client_list(ewmh_state->ewmh_c, ewmh_state->screen.screen_i,
                                     ewmh_state->clients.len, list);

   cookie = xcb_ewmh_set_client_list_stacking_checked(ewmh_state->ewmh_c, ewmh_state->screen.screen_i,
                                                      ewmh_state->clients.len, list);
   
   free(list);
   return 0;
}

int ewmh_update_numberOfDesktops(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   xcb_void_cookie_t cookie = xcb_ewmh_set_number_of_desktops(ewmh_state->ewmh_c, ewmh_state->screen.screen_i,
                                                              ewmh_state->desktops.len);
   return 0;
}

int ewmh_update_desktopGeometry(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   xcb_void_cookie_t cookie = xcb_ewmh_set_desktop_geometry(ewmh_state->ewmh_c, ewmh_state->screen.screen_i,
                                                            ewmh_state->screen.screen.width_in_pixels, ewmh_state->screen.screen.height_in_pixels);
   return 0;
}

int ewmh_update_desktopViewport(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   xcb_ewmh_coordinates_t *viewports = calloc(ewmh_state->desktops.len, sizeof(xcb_ewmh_coordinates_t));
   for(int i=0; i < ewmh_state->desktops.len; i++) {
      viewports[i].x = 0;
      viewports[i].y = 0;
   }
   xcb_void_cookie_t cookie = xcb_ewmh_set_desktop_viewport(ewmh_state->ewmh_c, ewmh_state->screen.screen_i,
                                                            ewmh_state->desktops.len, viewports);
   free(viewports);
   return 0;
}

int ewmh_update_currentDesktop(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   xcb_void_cookie_t cookie = xcb_ewmh_set_current_desktop(ewmh_state->ewmh_c, ewmh_state->screen.screen_i,
                                                           ewmh_state->desktops.current);
   return 0;
}

int ewmh_update_desktopNames(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   int len = 0;
   for(int i=0; i < ewmh_state->desktops.len; i++) {
      len += strlen(ewmh_state->desktops.desktops[i].name) + 1;
   }
   char *names = calloc(len, sizeof(char));
   int j = 0, k = 0;
   for(int i=0; i < len; i++) {
      if(ewmh_state->desktops.desktops[j].name[k]) {
         names[i] = ewmh_state->desktops.desktops[j].name[k];
         k++;
      }
      else {
         names[i] = '\0';
         k = 0;
         j++;
      }
   }
   xcb_void_cookie_t cookie = xcb_ewmh_set_desktop_names(ewmh_state->ewmh_c, ewmh_state->screen.screen_i,
                                                         ewmh_state->desktops.len, names);
   free(names);
   return 0;
}

int ewmh_update_activeWindow(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   xcb_void_cookie_t cookie = xcb_ewmh_set_active_window_checked(ewmh_state->ewmh_c, ewmh_state->screen.screen_i,
                                                                 ewmh_state->active_win);
   return 0;
}

int ewmh_update_workArea(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   xcb_ewmh_geometry_t *workareas = calloc(ewmh_state->desktops.len, sizeof(xcb_ewmh_coordinates_t));
   for(int i=0; i < ewmh_state->desktops.len; i++) {
      workareas[i] = ewmh_state->desktops.desktops[i].geom;
   }
   xcb_void_cookie_t cookie = xcb_ewmh_set_workarea(ewmh_state->ewmh_c, ewmh_state->screen.screen_i,
                                                    ewmh_state->desktops.len, workareas);
   free(workareas);
   return 0;
}

int ewmh_update_supportingWmCheck(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
	ewmh_state->supporting_win = xcb_generate_id(c);
	xcb_create_window(c, XCB_COPY_FROM_PARENT, ewmh_state->supporting_win, ewmh_state->root,
                     -1, -1, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_ONLY, XCB_COPY_FROM_PARENT, XCB_NONE, NULL);
   xcb_intern_atom_cookie_t atom_cookie   = xcb_intern_atom(c, 0, strlen("_NET_WM_NAME"), "_NET_WM_NAME");
   xcb_intern_atom_reply_t *atom_reply    = xcb_intern_atom_reply(c, atom_cookie, NULL);

   xcb_void_cookie_t cookie = xcb_ewmh_set_supporting_wm_check(ewmh_state->ewmh_c, ewmh_state->root, ewmh_state->supporting_win);
   xcb_ewmh_set_supporting_wm_check(ewmh_state->ewmh_c, ewmh_state->supporting_win, ewmh_state->root);
   xcb_ewmh_set_supporting_wm_check(ewmh_state->ewmh_c, ewmh_state->supporting_win, ewmh_state->supporting_win);
   xcb_ewmh_set_wm_name(ewmh_state->ewmh_c, ewmh_state->supporting_win, 
                        strlen(ewmh_state->wm_name), ewmh_state->wm_name);
   return 0;
}

int ewmh_update_virtualRoots(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   xcb_window_t *list = calloc(ewmh_state->clients.len, sizeof(client_t));
   for(int i=0; i < ewmh_state->clients.len; i++) {
      list[i] = (ewmh_state->clients.clients[i])->win;
   }

   xcb_void_cookie_t cookie = xcb_ewmh_set_virtual_roots(ewmh_state->ewmh_c, ewmh_state->screen.screen_i,
                                                         ewmh_state->virtual_roots.len, list);
   return 0;
}

int ewmh_update_showingDesktop(xcb_connection_t *c, ewmh_state_t *ewmh_state) {
   ewmh_state->showing_desktop = !ewmh_state->showing_desktop;
   xcb_void_cookie_t cookie = xcb_ewmh_set_showing_desktop(ewmh_state->ewmh_c, ewmh_state->screen.screen_i,
                                                           ewmh_state->showing_desktop);
   return 0;
}



  ////////////////////////////////////////////////////
 // Handle ewmh client messages                    //
////////////////////////////////////////////////////
void ewmh_handle_closeWindow(xcb_connection_t *c, xcb_client_message_event_t *msg, ewmh_state_t *ewmh_state) {
   unmanageWindow(c, msg->window, ewmh_state);
}

void ewmh_handle_moveResizeWindow(xcb_connection_t *c, xcb_client_message_event_t *msg, ewmh_state_t *ewmh_state){
   uint32_t grav_flags = msg->data.data32[0];
   if((grav_flags & (0xf << 8)) == 0xf) {
      return;
   }
   else {
      uint32_t x = msg->data.data32[1];
      uint32_t y = msg->data.data32[2];
      uint32_t width = msg->data.data32[3];
      uint32_t height = msg->data.data32[4];
      win_geom_t geom;
      geom.w = width;
      geom.h = height;
      switch(grav_flags & 0xf) {
         case 0:   
         case 1:   // NW  
            geom.x = x;
            geom.y = y;
            break;
         case 2:   // N 
            geom.x = x+(width/2);
            geom.y = y;
            break;
         case 3:   // NE
            geom.x = x+width;
            geom.y = y;
            break;
         case 4:   // W
            geom.x = x;
            geom.y = y+(height/2);
            break;
         case 5:   // CENTER
            geom.x = x+(width/2);
            geom.y = y+(height/2);
            break;
         case 6:   // E
            geom.x = x+width;
            geom.y = y+(height/2);
            break;
         case 7:   // SW
            geom.x = x;
            geom.y = y+height;
            break;
         case 8:   // S
            geom.x = x+(width/2);
            geom.y = y+height;
            break;
         case 9:   // SE
            geom.x = x+width;
            geom.y = y+height;
            break;
         case 10:  // STATIC
         default:
            geom.x = x;
            geom.y = y;
            break;
      }

      mvWindow(c, msg->window, geom, 1);
   }
}

void ewmh_handle_moveResize(xcb_connection_t *c, xcb_client_message_event_t *msg, ewmh_state_t *ewmh_state){
}

void ewmh_handle_restackWindow(xcb_connection_t *c, xcb_client_message_event_t *msg, ewmh_state_t *ewmh_state){
   if(ewmh_state->clients.len < 2) {
      return;
   }
   xcb_stack_mode_t stack_mode = msg->data.data32[2];
   clientList_t ref;
   client_t *target = NULL;
   int target_i;

   // get client
   for(int i=0; i<ewmh_state->clients.len; i++) {
      if(msg->window == ewmh_state->clients.clients[i]->win) {
         target = ewmh_state->clients.clients[i]; 
         target_i = i;
      }
   }
   if(!target) {
      return;
   }

   // create reference arr
   ref.len = ewmh_state->clients.len;
   memcpy(ref.clients, ewmh_state->clients.clients, ewmh_state->clients.len*sizeof(client_t*));

   // restack
   switch(stack_mode) {
      case XCB_STACK_MODE_ABOVE:
      case XCB_STACK_MODE_TOP_IF:
         for(int i=target_i; i < ewmh_state->clients.len-1; i++) {
            ewmh_state->clients.clients[i] = ref.clients[i+1];
         }
         ewmh_state->clients.clients[ewmh_state->clients.len-1] = target;
         break;
      case XCB_STACK_MODE_BELOW:
      case XCB_STACK_MODE_BOTTOM_IF:
      case XCB_STACK_MODE_OPPOSITE:
         ewmh_state->clients.clients[0] = target;
         for(int i=0; i < target_i; i++) {
            ewmh_state->clients.clients[i+1] = ref.clients[i];
         }
         break;
   }
   free(ref.clients);

   uint32_t values[] = { stack_mode };
   xcb_configure_window (c, msg->window, XCB_CONFIG_WINDOW_STACK_MODE, values);
}

void ewmh_handle_requestFrameExtents(xcb_connection_t *c, xcb_client_message_event_t *msg, ewmh_state_t *ewmh_state){
}
