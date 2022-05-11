CC = clang
CFLAGS = -g -Wall 
SANFLAGS = -Og -fsanitize=address
DBGFLAGS = -O0
LDFLAGS = -lxglue `pkg-config --cflags --libs xcb xcb-errors xcb-aux xcb-ewmh`

SOURCES = main.c handlers.c
LIBSOURCES = $(addprefix src/, ev_handle.c msg_handle.c win_aux.c evloop.c ewmh.c)

install : lib 
	mkdir -p /usr/include/libxglue
	install src/*.h /usr/include/libxglue
	install libxglue.so /usr/lib

lib: libxglue.so 

libxglue.so:
	$(CC) $(LDFLAGS) $(LIBSOURCES) -fPIC -shared -o $@

doc:
	mkdir -p doxygen
	doxygen doxygen/doxygen.conf

demo_wm:
	$(CC) $(LDFLAGS) demo/*.c -o $@


clean: 
	rm libxglue.so
