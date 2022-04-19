/**
 * @file
 */
#ifndef MSG_HANDLE_H
#define MSG_HANDLE_H

#include <uthash.h>
#include <xcb/xcb.h>

typedef enum {_END, NONE_ARG, INT_ARG, STR_ARG, CHAR_ARG, INT_ARG_OPTIONAL, STR_ARG_OPTIONAL, CHAR_ARG_OPTIONAL} paramType_et;

/**
 * struct {name} - int argument component of args_t
 */
typedef struct {
   int val;
   int set;
} int_arg_t;

/**
 * struct {name} - char argument component of args_t
 */
typedef struct {
   char val;
   int set;
} char_arg_t;

/**
 * struct {name} - str argument component of args_t
 */
typedef struct {
   char *val;
   int set;
} str_arg_t;

/**
 * struct {name} - none argument component of args_t
 */
typedef struct {
   int set;
} none_arg_t;

/**
 * struct {name} - parsed arguments passed to msg handlers
 */
typedef struct {
   int_arg_t *int_args;
   char_arg_t *char_args;
   str_arg_t *str_args;
   none_arg_t *none_args;
} args_t;

/**
 * struct {name} - socket message handler 
 */
typedef struct {
   /**
    * @{name} name of command triggerring this handler 
    */
   const char *cmd_name;
   /**
    * @{name} array of parameter types 
    */
   paramType_et *params;
   /**
    * @brief handler function pointer
    *
    * @param c 
    * @param params for passing params member of handler
    * @param args for passing actual arguments from socket message
    * @param wm_state state data passed to all handlers
    */
   char*(*handler)(xcb_connection_t *c, paramType_et *params, char *args, void *wm_state);
   UT_hash_handle hh;
} msg_handler_t;

/**
 * @brief default "focus" functionality handler
 */
char *focus_handler(xcb_connection_t *c, paramType_et *params, char *args, void *wm_state);

/**
 * @brief default "resize" funcitonality handler
 */
char *resize_handler(xcb_connection_t *c, paramType_et *params, char *args, void *wm_state);

/**
 * @brief array of default message handlers providing some basic functionalities
 */
static msg_handler_t default_msg_handlers[] = {
   {
      "focus",
      (paramType_et[]){CHAR_ARG, _END},
      focus_handler
   },
   {
      "resize",
      (paramType_et[]){CHAR_ARG, INT_ARG, _END},
      resize_handler
   }
};
#define default_msg_handlers_len sizeof(default_msg_handlers)/sizeof(default_msg_handlers[0])

args_t parse_args(paramType_et *params, char *args);

void free_args(args_t args);

/**
 * @brief creates hash table for executing ev_loop() 
 *
 * @param {name} array of statically defined msg handlers
 * @param handlers_len length of handlers[]
 * @return msg_handler_t hash table keyed by cmd_name
 */
msg_handler_t *setup_msg_handler_table(msg_handler_t handlers[], int handlers_len);

/**
 * @brief socket message handler
 *
 * @param c 
 * @param {name} &reference to message handler hash table pointer 
 * @param msg message recieved from socket
 * @param wm_state state data passed to all handlers
 * @return status message returned by called message handler 
 */
char *handle_msg(xcb_connection_t *c, msg_handler_t **handlers, char *msg, void *wm_state);

#endif
