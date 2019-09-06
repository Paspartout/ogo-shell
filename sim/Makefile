CFLAGS := -I ../components/hardware/include -I ../components/graphics -I ../main/include `sdl2-config --cflags` -O0 -g -DSIM=1 -Wall
LDLIBS := -lSDL2

# TODO: Header dependencies

TARGET := ogo-fm

SRC := ../main/src/main.o \
	../main/src/file_ops.o \
	../main/src/file_browser.o \
	../sim/hardware/display.o \
	../sim/hardware/event.o \
	../sim/hardware/keypad.o \
	../sim/hardware/backlight.o \
	../sim/hardware/system.o \
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