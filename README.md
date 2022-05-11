# libxglue
libxglue is a C library for easily writing X11 windows with a lower barrier of entry than using xlib or xcb. 
Notably, libxglue replaces neither of these and is meant to be used in tandem with xcb.

## features
### event loop
libxglue features an event loop that takes event handlers for handling XCB events and message handlers for handling UNIX socket messages. The latter is intended for herbstluftwm/bspwm style keybinds and configuration via sending socket messages from a shellscript and/or keybind daemon.

### aux
libxglue has an expanding list of auxillary functions for easing window manager development that mainly focus on reducing the boilerplate for common tasks like calculating the absolute position of a window.

## example project 
see demo/ in the repository, this is a simple but short window manager that features basic window borders, window dragging, and when used with sxhkd running via the included config keybinds for closing and resizing windows.

## doc 
Doxygen documentation can be generated with `make doc` and is located at doxygen/html/index.html

## install
`sudo make install` (make sure to get dependencies)
there's also a `lib` target if you'd just like to build libxglue.so

## dependencies
* libxcb
* xcb-util-error
* xcb-util-wm

