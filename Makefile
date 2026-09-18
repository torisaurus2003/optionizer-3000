CC = gcc
CFLAGS = -O2 -march=native -flto -Wall -Wextra -g $(shell pkg-config --cflags gtk4 gtk4-layer-shell-0)
LIBS = $(shell pkg-config --libs gtk4 gtk4-layer-shell-0)

optionizer-3000: optionizer-3000.c
	$(CC) $(CFLAGS) optionizer-3000.c -o optionizer-3000 $(LIBS)
clean:
	rm -f optionizer-3000
