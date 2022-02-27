CC = clang
CFLAGS = -g -Wall 
SANFLAGS = -Og -fsanitize=address
LDFLAGS = -lncurses `pkg-config --cflags --libs xcb xcb-errors xcb-aux`

SOURCES = $(addprefix src/, main.c ev_handle.c msg_handle.c)

test:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $@

san:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(SANFLAGS) -o $@

clean: 
	rm test san
