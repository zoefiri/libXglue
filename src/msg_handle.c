#include <string.h>
#include <stdio.h>
#include <uthash.h>
#include <xcb/xcb.h>

#include "msg_handle.h"

msg_handler_t *setup_msg_handler_table(msg_handler_t handlers[], int handlers_len) {
   msg_handler_t *table = NULL, *handler;

   for(int i=0; i < handlers_len; i++) {
      handler = (msg_handler_t*)malloc(sizeof *handler);
      handler->cmd_name = handlers[i].cmd_name;
      handler->params = handlers[i].params;
      handler->handler = handlers[i].handler;
      HASH_ADD_KEYPTR(hh, table, handler->cmd_name, strlen(handler->cmd_name), handler);
   }
   return table;
}

char *ws_trim(char *str) {
   if(str) {
      char *ws = " \n";
      int front = -1, back = -1, flag = 0;
      for(int i=0; str[i] != '\0'; i++) {
         for(int j=0; ws[j] != '\0'; j++) {
            if(str[i] == ws[j]) {
               flag = 1;
               break;
            }
         }
         if(!flag){
            if(front < 0) front = i;
            back = i;
         }
         flag = 0;
      }
      if(front < 0 || back < 0) {
         return "";
      }
      char *ret = calloc(back-front+1, sizeof(char));
      for(int i=0; i < back-front; i++) {
         ret[i] = str[i+front];
      }
      return ret;
   }
   return NULL;
}

int valid_dec_int(char *str) {
   if(*str == '-') {
      str++;
   }
   while(*str != '\0') {
      if(*str < '0' || *str > '9') return 0;
      str++;
   }
   return 1;
}
int validate_args(int *params, char *args) {
   char *tok = strtok(args, " \n");
   int optional_flag = 0;
   int i;
   for(i=0; params[i] && tok; i++) {
      switch(params[i]) {
         case NONE_ARG:
            return 1;
            break;

         case INT_ARG_OPTIONAL:
            optional_flag = 1;
         case INT_ARG:
            if(valid_dec_int(tok)) optional_flag = 0;
            else if(!optional_flag) return 0;
            break;

         case STR_ARG_OPTIONAL:
            optional_flag = 1;
         case STR_ARG:
            if(strlen(tok)) optional_flag = 0;
            break;

         case CHAR_ARG_OPTIONAL:
            optional_flag = 1;
         case CHAR_ARG:
            if(strlen(tok) == 1) optional_flag = 0;
            else if(!optional_flag) return 0;
            break;
      }
      if(!optional_flag) {
         tok = strtok(NULL, " \n");
      }
      else {
         optional_flag = 0;
      }
   }
   if(tok) {
      return 0;
   }
   else if(params[i]) {
      for(;params[i] != 0; i++) {
         if(params[i] < INT_ARG_OPTIONAL && params[i] != NONE_ARG) {
            return 0;
         }
      }
   }
   return 1;
}

char *focus_handler(xcb_connection_t *c, int *params, char *args, void *wm_state) {
   if(!validate_args(params, args)) {
      return("incorrect arguments");
   }
   return NULL;
}
char *resize_handler(xcb_connection_t *c, int *params, char *args, void *wm_state) {
   if(!validate_args(params, args)) {
      return("incorrect arguments");
   }
   return NULL;
}

char *handle_msg(xcb_connection_t *c, msg_handler_t **handlers, char *msg, void *wm_state) {
   msg_handler_t *handler;

   char *tokenize = strdup(msg);
   char *args, *cmd = strtok(tokenize, " \n");
   if(cmd != NULL) {
      args = strtok(NULL, " \n");
      if(args != NULL){
         printf("%d AAA\n", strlen(cmd));
         args = msg+strlen(cmd);
         printf("%s AAA,\n", args);
      }
   }
   else {
      return NULL;
   }

   HASH_FIND_STR(*handlers, cmd, handler);
   if(handler) {
      return((*handler->handler)(c, handler->params, args, wm_state));
   }
   else {
      return "cmd not found";
   }
}
