#include <xcb/xcb.h>
#include <stdlib.h>

xcb_translate_coordinates_reply_t  *calc_absolute_pos(xcb_connection_t *c, xcb_window_t *win, xcb_window_t *root, int x, int y) {
   xcb_query_tree_cookie_t reply_cookie   =  xcb_query_tree(c, *win);
   xcb_query_tree_reply_t  *win_tree      =  xcb_query_tree_reply(c, reply_cookie, NULL);

   xcb_translate_coordinates_cookie_t  dst_coord_cookie =  xcb_translate_coordinates(c, *win, win_tree->parent, x, y);
   xcb_translate_coordinates_reply_t   *dst_coord       =  xcb_translate_coordinates_reply(c, dst_coord_cookie, NULL);

   int            x_arg    = dst_coord->dst_x;
   int            y_arg    = dst_coord->dst_y;
   xcb_window_t   parent   = win_tree->parent;
   if (win_tree->parent == *root) {
      free(win_tree);
      return dst_coord;
   }
   else {
      free(dst_coord);
      free(win_tree);
      return calc_absolute_pos(c, &parent, root, x_arg, y_arg);
   }
   free(win_tree);
   free(dst_coord);
}
