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

// Application level code
#include <gbuf.h>
#include <graphics.h>
#include <OpenSans_Regular_11X12.h>
#include <tf.h>
#include <event.h>

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


void
app_main(void)
{
	display_init();
    keypad_init();
	event_init();

	tf_t *font = tf_new(&font_OpenSans_Regular_11X12, 0x0000, 0, TF_ALIGN_CENTER);
	//tf_t *font_white = tf_new(&font_OpenSans_Regular_11X12, 0xFFFF, 0, TF_ALIGN_CENTER);
	fill_rectangle(fb, (rect_t){.x = 0, .y = 0, .width = DISPLAY_WIDTH, .height = 16}, 0xFFFF);
	tf_draw_str(fb, font, "ogo-fm", (point_t){.x = 40, .y = 3});
	display_update();

	bool quit = false;
	int selection = 0;
	event_t event;
	draw_selection(selection);
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
					printf("UP!\n");
					if(--selection < 0) {
						selection = 0;
					}
					draw_selection(selection);
					break;
				case KEYPAD_DOWN:
					printf("DOWN!\n");
					if(++selection > 5) {
						selection = 5;
					}
					draw_selection(selection);
					break;
				case KEYPAD_RIGHT:
					printf("RIGHT!\n");
					break;
				case KEYPAD_LEFT:
					printf("LEFT!\n");
					break;
				}
			}
				break;
			default:
				printf("OTHER!\n");
				break;
		}
	}

	display_poweroff();
}

#ifdef SIM

int main(int argc, char const* argv[])
{
	app_main();
	return 0;
}

#endif