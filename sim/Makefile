CC := clang
CXX := clang++
CFLAGS := $(CFLAGS) -Os -g -I ../components/hardware/include -I ../components/graphics -I ../main/include -I ../components/acodecs/include -I ../components/acodecs/src/xmplite -I ../components/acodecs/src/gme/gme `sdl2-config --cflags` -DSIM=1 -Wall -DLIBXMP_CORE_PLAYER
CXXFLAGS := -DVGM_YM2612_NUKED
LDLIBS := -lSDL2 -lm

# TODO: Header dependencies

TARGET := ogo-shell

LIBXM_SRC := $(wildcard ../components/acodecs/src/xmplite/*.c) $(wildcard ../components/acodecs/src/xmplite/loaders/*.c)
GME_SRC := $(wildcard ../components/acodecs/src/gme/gme/*.cpp)

SRC := ../main/src/main.o \
	../main/src/file_ops.o \
	../main/src/file_browser.o \
	../main/src/audio_player.o \
	../main/src/image_viewer.o \
	../main/src/emulator_launcher.o \
	../main/src/ui.o \
	../main/src/status_bar.o \
	../main/src/str_utils.o \
	../sim/hardware/display.o \
	../sim/hardware/event.o \
	../sim/hardware/keypad.o \
	../sim/hardware/backlight.o \
	../sim/hardware/system.o \
	../sim/hardware/sdcard.o \
	../sim/hardware/audio.o \
	../sim/hardware/battery.o \
	../sim/hardware/settings.o \
	../components/acodecs/src/acodecs.o \
	../components/acodecs/src/stb_vorbis.o \
	../components/acodecs/src/dr_mp3.o \
	../components/acodecs/src/dr_wav.o \
	../components/acodecs/src/dr_flac.o \
	../components/hardware/src/gbuf.o \
	../components/graphics/graphics.o \
	../components/graphics/tf.o \
	../components/graphics/OpenSans_Regular_11X12.o \
	$(LIBXM_SRC) \

OBJ := ${SRC:.c=.o} ${GME_SRC:.cpp=.o}

all: $(TARGET)

# LDFLAGS := -fsanitize=undefined -fno-omit-frame-pointer -fsanitize=address


obj:
	echo ${OBJ}

$(TARGET): ${OBJ}
	@echo LD $@
	@${CXX} ${LDFLAGS} -o $@ $^ ${LDLIBS}

clean:
	rm -f ${TARGET} ${OBJ}

.PHONY: clean
