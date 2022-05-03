#include <string.h>
#include <stdio.h>
#include <uthash.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "msg_handle.h"
#include "win_aux.h"
#include "wm_state.h"

msg_handler_t *setup_msg_handler_table(msg_handler_t handlers[], int handlers_len) {
   msg_handler_t *table = NULL, *handler;

   printf("enterin %d\n", handlers_len);
   for(int i=0; i < handlers_len; i++) {
      handler = (msg_handler_t*)malloc(sizeof *handler);
      handler->cmd_name = handlers[i].cmd_name;
      handler->params = handlers[i].params;
      handler->handler = handlers[i].handler;
      printf("guh..\n");
      printf("%s\n", handlers[i].cmd_name);
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

void free_args(args_t args) {
   if(args.none_args) free(args.none_args);
   if(args.int_args)  free(args.int_args);
   if(args.char_args) free(args.char_args);
   if(args.str_args) {
      for(int i=0; i < args.str_args_len; i++) {
         if(args.str_args[i].val) free(args.str_args[i].val);
      }
      free(args.str_args);
   }  
}

args_t parse_args(paramType_et *params, char *args) {
   args_t parsed;
   int int_c = 0, char_c = 0, str_c = 0, none_c = 0; 
   for(int i=0; params[i] != _END; i++) {
      switch(params[i]) {
         case NONE_ARG:
            none_c++;
            break;

         case INT_ARG_OPTIONAL:
         case INT_ARG:
            int_c++;
            break;

         case STR_ARG_OPTIONAL:
         case STR_ARG:
            str_c++;
            break;

         case CHAR_ARG_OPTIONAL:
         case CHAR_ARG:
            char_c++;
            break;
      }
   }
   parsed.int_args = calloc(int_c, sizeof(int_arg_t));
   parsed.char_args = calloc(char_c, sizeof(char_arg_t));
   parsed.str_args = calloc(str_c, sizeof(str_arg_t));
   parsed.none_args = calloc(str_c, sizeof(none_arg_t));
   parsed.str_args_len = str_c;

   int int_p = 0, char_p = 0, str_p = 0, none_p = 0; 
   int optional_flag = 0;
   char *tok = strtok(args, " \n");
   for(int i=0; params[i] && tok; i++) {
      switch(params[i]) {
         case NONE_ARG:
            parsed.none_args[none_p].set = 1;
            break;

         case INT_ARG_OPTIONAL:
            optional_flag = 1;
         case INT_ARG:
            if(valid_dec_int(tok)) {
               parsed.int_args[int_p].val = strtol(tok, NULL, 10);
               parsed.int_args[int_p].set = 1;
            }
            else if(optional_flag) {
               parsed.int_args[int_p].val = 0;
               parsed.int_args[int_p].set = 0;
            }
            int_p++;
            break;

         case STR_ARG_OPTIONAL:
            optional_flag = 1;
         case STR_ARG:
            if(strlen(tok)) {
               parsed.str_args[str_p].val = strdup(tok);
               parsed.str_args[str_p].set = 1;
            }
            else if(optional_flag) {
               parsed.str_args[str_p].val = NULL;
               parsed.str_args[str_p].set = 0;
            }
            break;

         case CHAR_ARG_OPTIONAL:
            optional_flag = 1;
         case CHAR_ARG:
            if(strlen(tok) == 1) {
               parsed.char_args[char_p].val = tok[0];
               parsed.char_args[char_p].set = 1;
            }
            else if(optional_flag) {
               parsed.char_args[char_p].val = '\0';
               parsed.char_args[char_p].set = 0;
            }
            char_p++;
            break;
      }
      if(!optional_flag) {
         tok = strtok(NULL, " \n");
      }
      else {
         optional_flag = 0;
      }
   }
   return parsed;
}


char *handle_msg(xcb_connection_t *c, msg_handler_t **handlers, char *msg, void *wstate) {
   msg_handler_t *handler;

   char *tokenize = strdup(msg);
   char *args, *cmd = strtok(tokenize, " \n");
   if(cmd != NULL) {
      args = strtok(NULL, " \n");
      if(args != NULL){
         args = msg+strlen(cmd);
      }
   }
   else {
      free(tokenize);
      return NULL;
   }

   HASH_FIND_STR(*handlers, cmd, handler);
   printf("looking for cmd |%s|, 0x%x\n", cmd, handler);
   if(handler) {
      char *response = (*handler->handler)(c, handler->params, args, wstate);
      xcb_flush(c);
      free(tokenize);
      return response;
   }
   else {
      free(tokenize);
      return "cmd not found";
   }
}
