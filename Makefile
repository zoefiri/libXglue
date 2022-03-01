CC = clang
CFLAGS = -g -Wall 
SANFLAGS = -Og -fsanitize=address
LDFLAGS = -lncurses `pkg-config --cflags --libs xcb xcb-errors xcb-aux`

SOURCES = $(addprefix src/, ev_handle.c msg_handle.c win_aux.c evloop.c) main.c

test:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $@

san:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(SANFLAGS) -o $@

doc:
	doxygen doxygen/doxygen.conf

clean: 
	rm test san
