#include <string.h>
#include <uthash.h>
#include <xcb/xcb.h>

#define INT_ARG 0
#define STR_ARG 1
#define CHAR_ARG 2

typedef struct {
   const char *cmd_name;
   int *args;
   void(*handler)(xcb_connection_t *c, char **args);
   UT_hash_handle hh;
} msg_handler_t;

void focus_handler(xcb_connection_t *c, char **args) {
   return;
}
msg_handler_t *default_msg_handler = (msg_handler_t[]){
   {
      "focus",
      (int[]){INT_ARG},
      focus_handler,
      NULL
   },
   NULL
};

msg_handler_t *setup_msg_handler_table(msg_handler_t *handler_list) {
   while(handler_list != NULL) {

   }
}

int handle_msg(xcb_connection_t *c, msg_handler_t **handlers, char *msg) {
   msg_handler_t *handler;

   char *d = " ";
   char **args = calloc(sizeof(char*), 100);
   char *arg = strtok(msg, d);
   char *cmd = arg;
   if(cmd != NULL) {
      for(int i=0; (arg = strtok(NULL, d)) != NULL; i++) {
         *(args+i) = arg;
         *(args+i+1) = NULL;
      }
   }
   else {
      return 1;
   }

   HASH_FIND_STR(*handlers, cmd, handler);
   (*handler->handler)(c, args);
   return 0;
}

int main() {
   return 0;
}
