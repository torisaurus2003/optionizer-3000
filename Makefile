CC = gcc
CFLAGS = -O2 -march=native -flto $(shell pkg-config --cflags gtk4 gtk4-layer-shell-0)
LIBS = $(shell pkg-config --libs gtk4 gtk4-layer-shell-0)

power-menu: power-menu.c
	$(CC) $(CFLAGS) power-menu.c -o power-menu $(LIBS)
clean:
	rm -f power-menu
