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

args_t parse_args(int *params, char *args) {
   args_t parsed;
   int int_c = 0, char_c = 0, str_c = 0, none_c = 0; 
   for(int i=0; params[i] != 0; i++) {
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

char *focus_handler(xcb_connection_t *c, int *params, char *args, void *wm_state) {
   args_t parsed = parse_args(params, args);
   return NULL;
}
char *resize_handler(xcb_connection_t *c, int *params, char *args, void *wm_state) {
   wm_state_t *wstate = (wm_state_t*)wm_state;
   args_t parsed = parse_args(params, args);
   int win_geom[] = {0, 0, 0, 0};

   if(parsed.char_args[0].set && parsed.int_args[0].set && wstate->active_win) {
      xcb_translate_coordinates_reply_t *coords    =  calc_absolute_pos(c, &wstate->active_win, &wstate->root, 0, 0);
      xcb_get_geometry_cookie_t geom_cookie        =  xcb_get_geometry(c, wstate->active_win);
      xcb_get_geometry_reply_t *geom               =  xcb_get_geometry_reply(c, geom_cookie, NULL);
      win_geom[0] = coords->dst_x;  // x
      win_geom[1] = coords->dst_y;  // y
      win_geom[2] = geom->width;    // width
      win_geom[3] = geom->height;   // height
      switch(parsed.char_args[0].val) {
         case 'l':
            win_geom[0] += -1*parsed.int_args[0].val;
            win_geom[2] += parsed.int_args[0].val;
            break;
         case 'r':
            win_geom[2] += parsed.int_args[0].val;
            break;
         case 'u':
            win_geom[1] += -1*parsed.int_args[0].val;
            win_geom[3] += parsed.int_args[0].val;
            break;
         case 'd':
            win_geom[3] += parsed.int_args[0].val;
            break;
         default:
            return "invalid input";
            break;
      }
      xcb_configure_window(c, wstate->active_win, XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT,
                                                  win_geom); 
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
