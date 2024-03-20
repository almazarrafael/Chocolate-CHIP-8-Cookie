CC = gcc
CFLAGS = -g -lSDL2 -lSDL2main
RM = rm -f


default: all

all: cc8c

cc8c: main.c
	$(CC) -o cc8c main.c graphics.c chip8_core.c $(CFLAGS)

clean veryclean:
	$(RM) cc8c