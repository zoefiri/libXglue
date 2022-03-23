CC = clang
CFLAGS = -g -Wall 
SANFLAGS = -Og -fsanitize=address
DBGFLAGS = -O0
LDFLAGS = -lncurses `pkg-config --cflags --libs xcb xcb-errors xcb-aux xcb-ewmh`

SOURCES = $(addprefix src/, ev_handle.c msg_handle.c win_aux.c evloop.c ewmh.c) main.c

test:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $@

san:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(SANFLAGS) -o $@

dbg:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(DBGFLAGS) -o $@

doc:
	doxygen doxygen/doxygen.conf

clean: 
	rm test san dbg
