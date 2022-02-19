#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xproto.h>
#include <xcb/xcb_atom.h>

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
