#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <endian.h>

#include <display.h>

#include <gbuf.h>
#include <graphics.h>
#include <OpenSans_Regular_11X12.h>
#include <tf.h>

#include <SDL2/SDL.h>

int ls(const char* path, tf_t *font) {
	DIR *dir = opendir(path);
	if (dir == NULL) {
		perror("opendir");
		return -1;
	}

	errno = 0;
	int yoff = 0;
	for(struct dirent* entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
		if (errno != 0) {
			perror("readdir");
			break;
		}
		// printf("%c - %s\n", entry->d_type == DT_DIR ? 'd' : 'f', entry->d_name);
		tf_draw_str(fb, font, entry->d_name, (point_t){.x = 10, .y = 20 + yoff});
		yoff += 20;
	}

	closedir(dir);
}

typedef enum Event {
	NONE,

	KEY_UP,
	KEY_RIGHT,
	KEY_DOWN,
	KEY_LEFT,
	KEY_A,
	KEY_B,
	KEY_START,
	KEY_SELECT,
	KEY_MENU,
	KEY_VOLUME,

	QUIT,
	RESIZE,
} Event;

#define RETURN_EVENT(ev) *event = ev; return 0;



int wait_event(Event *event) {
	static SDL_Event e;

	for(;;) {
		if (!SDL_WaitEvent(&e)) {
			return -1;
		}

		switch (e.type) {
			case SDL_KEYDOWN:
				// TODO: Map SDL Key events to Event
				switch(e.key.keysym.sym) {
				case SDLK_UP:
					RETURN_EVENT(KEY_UP);
					break;
				case SDLK_DOWN:
					RETURN_EVENT(KEY_DOWN);
					break;
				case SDLK_RIGHT:
					RETURN_EVENT(KEY_RIGHT);
					break;
				case SDLK_LEFT:
					RETURN_EVENT(KEY_LEFT);
					break;
				}
				break;
			case SDL_QUIT:
				RETURN_EVENT(QUIT);
				break;
			case SDL_WINDOWEVENT:
				RETURN_EVENT(RESIZE);
				break;
			default:
				break;
		}

	}
}

const char* entries[] = {
	"Dir 1",
	"Dir 2",
	"File 1",

	"File 2",
	"File 3",
	"File 4",
};

void draw_selection(int selection) {
	for (int i = 0; i < 6; i++) {
		const uint16_t bg_color = i == selection ? 0xDB60 : 0x4A69;
		// Draw background of entry
		const int y_start = 20;
		const int height = 16;
		fill_rectangle(fb, (rect_t){.x = 0, .y = y_start + i * height, .width = DISPLAY_WIDTH, .height = height}, bg_color);
		// Draw text on top
		tf_t *font_white = tf_new(&font_OpenSans_Regular_11X12, 0xFFFF, 0, TF_ALIGN_CENTER);
		tf_draw_str(fb, font_white, entries[i], (point_t){.y = y_start + i * height + 2, .x = 3});
	}
	display_update();
}


int main(int argc, char const* argv[])
{
	display_init();

	tf_t *font = tf_new(&font_OpenSans_Regular_11X12, 0x0000, 0, TF_ALIGN_CENTER);
	tf_t *font_white = tf_new(&font_OpenSans_Regular_11X12, 0xFFFF, 0, TF_ALIGN_CENTER);
	fill_rectangle(fb, (rect_t){.x = 0, .y = 0, .width = DISPLAY_WIDTH, .height = 16}, 0xFFFF);
	tf_draw_str(fb, font, "ogo-fm", (point_t){.x = 40, .y = 3});
	display_update();

	bool quit = false;
	int selection = 0;
	Event event;
	draw_selection(selection);
	while(!quit) {
		// Handle inputs
		wait_event(&event);
		switch(event) {
			case QUIT:
				quit = true;
				break;
			case RESIZE:
				display_update();
				break;
			case KEY_UP:
				printf("UP!\n");
				if(--selection < 0) {
					selection = 0;
				}
				draw_selection(selection);
				break;
			case KEY_DOWN:
				printf("DOWN!\n");
				if(++selection > 5) {
					selection = 5;
				}
				draw_selection(selection);
				break;
			case KEY_RIGHT:
				printf("RIGHT!\n");
				break;
			case KEY_LEFT:
				printf("LEFT!\n");
				break;
		}
	}

	display_deinit();
	return 0;
}
