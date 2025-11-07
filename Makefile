SRCMODULES = list.c
OBJMODULES = $(SRCMODULES:.c=.o)
CC = gcc
CFLAGS = -g -Wall -std=c99 -pedantic

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

prog: main.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o prog deps.mk
