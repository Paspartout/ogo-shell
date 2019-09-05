CFLAGS := -I ../components/hardware/include -I ../components/graphics -I ../main `sdl2-config --cflags` -O1 -g -DSIM=1 -Wall -Werror
LDLIBS := -lSDL2

# TODO: Header dependencies

TARGET := ogo-fm

SRC := ../main/main.o \
	../sim/hardware/display.o \
	../sim/hardware/event.o \
	../sim/hardware/keypad.o \
	../components/hardware/src/gbuf.o \
	../components/graphics/graphics.o \
	../components/graphics/tf.o \
	../components/graphics/OpenSans_Regular_11X12.o

OBJ := ${SRC:.c=.o}

$(TARGET): ${OBJ}
	@echo LD $@
	@${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

clean:
	rm -f ${TARGET} ${OBJ}

.PHONY: clean