#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <endian.h>

// Driver/Hardware
#include <display.h>
#include <keypad.h>
#include <system.h>
#include <backlight.h>

// Application level code
#include <gbuf.h>
#include <graphics.h>
#include <OpenSans_Regular_11X12.h>
#include <tf.h>
#include <event.h>

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

#ifndef APP_NAME
#define APP_NAME "ogo-fm"
#endif

#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

const char* entries[] = {
	"ABCDEFG",
	"Dir 2",
	"File 1",
	"File 2",
	"File 3",
	"File 4",
	"File 5",
	"File 6",
	"File 7",
	"File 8",
	"File 9",
	"File 1000",
	"File 1123123",
	"Dir 1",
	"Dir 2",
	"File 1",
	"File 2",
	"File 3",
	"File 4",
	"File 5",
	"File 6",
	"File 7",
	"File 8",
	"File 9",
	"File 1000",
	"File 1123123",
	"ABCDEFG",
	"Dir 2",
	"File 1",
	"File 2",
	"File 3",
	"File 4",
	"File 5",
	"File 6",
	"File 7",
	"File 8",
	"File 9",
	"File 1000",
	"File 1123123",
	"Dir 1",
	"Dir 2",
	"File 1",
	"File 2",
	"File 3",
	"File 4",
	"File 5",
	"File 6",
	"File 7",
	"File 8",
	"File 9",
	"File 1000",
	"File 1123123",
	"File 1",
	"File 2",
	"File 3",
	"File 4",
	"File 5",
	"File 6",
	"File 7",
	"File 8",
	"File 9",
	"File 1000",
	"File 1123123",
};

const int max_lines = 13;

void draw_selection(int selection, int scroll) {
	// printf("Array lenght: %ld\n", ARRAY_LENGTH(entries));
	printf("selection: %d, scroll: %d\n", selection, scroll);

	tf_t *font_white = tf_new(&font_OpenSans_Regular_11X12, 0xFFFF, 0, TF_ALIGN_CENTER); // TODO: Load once

	for (int i = scroll; i < scroll+max_lines; i++) {
		const uint16_t bg_color = i == selection ? 0xDB60 : 0x4A69;
		// Draw background of entry
		const int y_start = 20;
		const int line_height = 16;

		const int j = (i - scroll); // in [0,13]

		if (i < ARRAY_LENGTH(entries)) {
			fill_rectangle(fb, (rect_t){.x = 0, .y = y_start + j * line_height, .width = DISPLAY_WIDTH, .height = line_height}, bg_color);
			// Draw text on top
			printf("Drawing  i=%d\n", i);
			tf_draw_str(fb, font_white, entries[i], (point_t){.y = y_start + j * line_height + 2, .x = 3});
		}
	}
	tf_free(font_white);
	display_update();
}

static void app_init(void) {
	display_init();
    backlight_init();
    keypad_init();
	event_init();

	// TODO: Setup sdcard and display error message on failure
}

static void app_shutdown(void) {
	display_poweroff();
	reboot_to_firmware();
}


void
app_main(void)
{
	app_init();

	tf_t *font = tf_new(&font_OpenSans_Regular_11X12, 0x0000, 0, TF_ALIGN_CENTER);
	//tf_t *font_white = tf_new(&font_OpenSans_Regular_11X12, 0xFFFF, 0, TF_ALIGN_CENTER);
	fill_rectangle(fb, (rect_t){.x = 0, .y = 0, .width = DISPLAY_WIDTH, .height = 16}, 0xFFFF);
	tf_draw_str(fb, font, "ogo-fm " APP_VERSION, (point_t){.x = 3, .y = 3});
	display_update();

	bool quit = false;
	int selection = 0;
	int scroll = 0;
	event_t event;
	draw_selection(selection, scroll);
	while(!quit) {
		// Handle inputs
		wait_event(&event);
		switch(event.type) {
			case EVENT_TYPE_QUIT:
				quit = true;
				break;
			case EVENT_TYPE_UPDATE:
				display_update();
				break;
			case EVENT_TYPE_KEYPAD:
			{
				switch(event.keypad.pressed) {
				case KEYPAD_UP:
					if(--selection < 0) {
						selection = 0;
					}
					if (selection - scroll < 0) {
						scroll--;
					}
					draw_selection(selection, scroll);
					break;
				case KEYPAD_DOWN:
					if (++selection > ARRAY_LENGTH(entries)-1) {
						selection = ARRAY_LENGTH(entries)-1;
					}
					if (selection - scroll > (max_lines-1)) {
						scroll++;
					}
					draw_selection(selection, scroll);
					break;
				case KEYPAD_RIGHT:
					break;
				case KEYPAD_LEFT:
					break;
				case KEYPAD_MENU:
					quit = true;
					break;
				}
			}
				break;
			default:
				printf("OTHER!\n");
				break;
		}
	}

	app_shutdown();
}

#ifdef SIM

int main(int argc, char const* argv[])
{
	app_main();
	return 0;
}

#endif