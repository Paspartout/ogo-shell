CFLAGS := -I hardware/src/ -I graphics/ `sdl2-config --cflags` -O1 -g
LDFLAGS := 
LDLIBS := -lSDL2

all: main

main: main.o hardware/src/gbuf.o graphics/graphics.o graphics/tf.o graphics/OpenSans_Regular_11X12.o

hardware/src/gbuf.o: hardware/src/gbuf.c
main.o: main.c


.PHONY: all
