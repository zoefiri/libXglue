#include <asm-generic/socket.h>
#include <signal.h>
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
#include "win_aux.h"

static volatile uint8_t global_cleanup_flag = 0;

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

void pass() {
   return;
}

void set_cleanup() {
   printf("CLEANUP SET\n");
   global_cleanup_flag = 1;
}

void p_focus(xcb_connection_t *c) {
   xcb_get_input_focus_cookie_t cookie = xcb_get_input_focus(c);
   xcb_get_input_focus_reply_t *reply = xcb_get_input_focus_reply(c, cookie, NULL);
   printf("current focus: 0x%x\t", reply->focus);
   free(reply);
}

int ev_loop(xcb_connection_t *c, ev_handler_t *ev_handlers, msg_handler_t **msg_handlers, ewmh_state_t *ewmh_state, void *wm_state) {
   signal(SIGINT, set_cleanup);
   signal(SIGTERM, set_cleanup);
   signal(SIGPIPE, pass);

   int sock_fd = setup_socket("/tmp/libxwm");
   int c_fd = xcb_get_file_descriptor(c);
   int client_fd;

   struct pollfd fds[2] = {{c_fd, POLLIN},{sock_fd, POLLIN}};

   xcb_generic_event_t *ev;
   char buff[2000];
   char *handler_response;
   while(1) {
      if(global_cleanup_flag) {
         cleanup(c, ewmh_state);
         exit(0);
      }
      poll(fds, 2, -1);

      if(fds[0].revents == POLLIN) {
         xcb_aux_sync(c);
         while((ev = xcb_poll_for_event(c))) {
            handle_ev(c, ev_handlers, ev, wm_state);
            printf("ev: %d\n", ev->response_type);
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
