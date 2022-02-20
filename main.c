#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xproto.h>
#include <xcb/xcb_atom.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "events.h"

int setup_socket(int port) {
   int sock_fd;
   int opt = 1;
   if((sock_fd = socket(AF_LOCAL, SOCK_STREAM, 0)) == 0){
      fprintf(stderr, "socket creation failed\n");
      return -1;
   }
   if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &opt, sizeof(opt)) != 0){
      fprintf(stderr, "failed to configure socket");
      return -1;
   }
   struct sockaddr_in addr = {AF_INET, htons(port), {INADDR_ANY}};
   if(bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      fprintf(stderr, "failed to bind to port %d, is it already in use?\n", 44693);
      return -1;
   }
   if(listen(sock_fd, 10) < 0) {
      fprintf(stderr, "socket listening failed");
      return -1;
   }
   return sock_fd;
}

int ev_loop(xcb_connection_t *c, handler_t *handlers) {
   int sock_fd = setup_socket(44693);
   int c_fd = xcb_get_file_descriptor(c);
   int client_fd;

   fd_set fds;
   FD_ZERO(&fds);
   FD_SET(c_fd, &fds);
   FD_SET(sock_fd, &fds);

   xcb_generic_event_t *ev;
   while(1) {
      select(3, &fds, NULL, NULL, NULL);

      if(FD_ISSET(c_fd, &fds)) {
         ev = xcb_wait_for_event(c);
         handlers[ev->response_type].fp(c, ev, NULL);
      }

      if(FD_ISSET(sock_fd, &fds)) {
         client_fd = accept(sock_fd, NULL, NULL);
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
   return 0;
}
