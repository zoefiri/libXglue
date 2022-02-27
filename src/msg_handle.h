#ifndef MSG_HANDLE_H
#define MSG_HANDLE_H

#include <uthash.h>
#include <xcb/xcb.h>

#define NONE_ARG 1
#define INT_ARG 2
#define STR_ARG 3
#define CHAR_ARG 4
#define INT_ARG_OPTIONAL 5
#define STR_ARG_OPTIONAL 6
#define CHAR_ARG_OPTIONAL 7

typedef struct {
   const char *cmd_name;
   int *params;
   char*(*handler)(xcb_connection_t *c, int *params, char *args, void *wm_state);
   UT_hash_handle hh;
} msg_handler_t;

int validate_args(int *params, char *args);

char *focus_handler(xcb_connection_t *c, int *params, char *args, void *wm_state);
char *resize_handler(xcb_connection_t *c, int *params, char *args, void *wm_state);
static msg_handler_t default_msg_handlers[] = {
   {
      "focus",
      (int[]){CHAR_ARG, 0},
      focus_handler
   },
   {
      "resize",
      (int[]){CHAR_ARG, INT_ARG, 0},
      resize_handler
   }
};
#define default_msg_handlers_len sizeof(default_msg_handlers)/sizeof(default_msg_handlers[0])

msg_handler_t *setup_msg_handler_table(msg_handler_t handlers[], int handlers_len);
char *handle_msg(xcb_connection_t *c, msg_handler_t **handlers, char *msg, void *wm_state);

#endif
