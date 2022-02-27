#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xproto.h>
#include <xcb/xcb_atom.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <unistd.h>

#include "ev_handle.h"
#include "msg_handle.h"
#include "wm_state.h"

int setup_socket(char *sock_path) {
   int sock_fd;
   int opt = 1;
   if((sock_fd = socket(AF_LOCAL, SOCK_STREAM, 0)) == 0){
      fprintf(stderr, "socket creation failed\n");
      return -1;
   }
   if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0){
      fprintf(stderr, "failed to configure socket");
      return -1;
   }
   struct sockaddr_un addr;
   memset(&addr, 0, sizeof(addr));
   addr.sun_family = AF_LOCAL;
   snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", sock_path);
   unlink(sock_path);
   if(bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      fprintf(stderr, "failed to bind to socket %s, is it already in use?\n", sock_path);
      fprintf(stderr, "OOF: %s %d %d\n ", strerror(errno), errno, EINVAL);
      return -1;
   }
   if(listen(sock_fd, 10) < 0) {
      fprintf(stderr, "socket listening failed");
      return -1;
   }
   return sock_fd;
}

int ev_loop(xcb_connection_t *c, ev_handler_t *ev_handlers, msg_handler_t **msg_handlers, void *wm_state) {
   int sock_fd = setup_socket("/tmp/libxwm");
   int c_fd = xcb_get_file_descriptor(c);
   int client_fd;

   struct pollfd fds[2] = {{c_fd, POLLIN},{sock_fd, POLLIN}};

   xcb_generic_event_t *ev;
   char buff[2000];
   char *handler_response;
   while(1) {
      poll(fds, 2, -1);
      printf("uh\n");

      if(fds[0].revents == POLLIN) {
         xcb_aux_sync(c);
         while((ev = xcb_poll_for_event(c))) {
            handle_ev(c, ev_handlers, ev, wm_state);
         }
      }

      if(fds[1].revents == POLLIN) {
         client_fd = accept(sock_fd, NULL, wm_state);
         if(read(client_fd, buff, 2000)) {
            printf("receieved msg: \"%s\"\n", buff);
            if((handler_response = handle_msg(c, msg_handlers, buff, wm_state))) {
               send(client_fd, handler_response, strlen(handler_response), 0);
            }
         }
         else {
            printf("0 len msg recieved!\n");
            send(client_fd, "cringe", strlen("cringe"), 0 );
         }
      }
   }
   return 0;
}

int main() {
   xcb_connection_t        *c;
   xcb_screen_t            *screen;
   int                     screen_i;
   xcb_screen_iterator_t   iter;

   c     =   xcb_connect(NULL, &screen_i);
   iter  =   xcb_setup_roots_iterator (xcb_get_setup (c));

   //xcb doc code for getting last screen, not messing with multimonitor yet so I'm just gonna use this
   for (; iter.rem; --screen_i, xcb_screen_next (&iter)) { 
      if (screen_i == 0) {
         screen = iter.data;
         break;
      }
   }
   const uint32_t select_input_val[] =
   {
      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
         | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW
         | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE
         | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
         | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_POINTER_MOTION
   };
   xcb_change_window_attributes(c,
         screen->root,
         XCB_CW_EVENT_MASK, select_input_val
         );

   xcb_aux_sync(c);

   if (xcb_poll_for_event(c) != NULL)
   {
      perror("another window manager is already running");
      exit(1);
   }
   xcb_flush(c);

   wm_state_t wm_state;

   wm_state.win_geoms = calloc(100, sizeof(win_geom_t));
   msg_handler_t *msg_handlers = setup_msg_handler_table(default_msg_handlers, 2);
   ev_loop(c, default_ev_handlers, &msg_handlers, &wm_state);
   return 0;
}
