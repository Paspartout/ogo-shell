CFLAGS := -O2 -I ../components/hardware/include -I ../components/graphics -I ../main/include -I ../components/acodecs/include -I ../components/acodecs/src/xmplite `sdl2-config --cflags` -O0 -g -DSIM=1 -Wall -DLIBXMP_CORE_PLAYER
LDLIBS := -lSDL2 -lm

# TODO: Header dependencies

TARGET := ogo-fm

LIBXM_SRC := $(wildcard ../components/acodecs/src/xmplite/*.c) $(wildcard ../components/acodecs/src/xmplite/loaders/*.c)

SRC := ../main/src/main.o \
	../main/src/file_ops.o \
	../main/src/file_browser.o \
	../main/src/audio_player.o \
	../main/src/ui.o \
	../sim/hardware/display.o \
	../sim/hardware/event.o \
	../sim/hardware/keypad.o \
	../sim/hardware/backlight.o \
	../sim/hardware/system.o \
	../sim/hardware/sdcard.o \
        ../sim/hardware/audio.o \
        ../components/acodecs/src/acodecs.o \
        ../components/acodecs/src/stb_vorbis.o \
	../components/acodecs/src/dr_mp3.o \
	../components/acodecs/src/dr_wav.o \
	../components/acodecs/src/dr_flac.o \
	../components/hardware/src/gbuf.o \
	../components/graphics/graphics.o \
	../components/graphics/tf.o \
        ../components/graphics/OpenSans_Regular_11X12.o \
        $(LIBXM_SRC)

OBJ := ${SRC:.c=.o}

all: $(TARGET)

$(TARGET): ${OBJ}
	@echo LD $@
	@${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

clean:
	rm -f ${TARGET} ${OBJ}

.PHONY: clean
