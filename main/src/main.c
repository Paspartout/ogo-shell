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
#include <event.h>

#include <gbuf.h>
#include <graphics.h>
#include <OpenSans_Regular_11X12.h>
#include <tf.h>

#include <file_browser.h>

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

#ifndef APP_NAME
#define APP_NAME "ogo-fm"
#endif

#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

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

	// Draw title
	tf_t *font = tf_new(&font_OpenSans_Regular_11X12, 0x0000, 0, TF_ALIGN_CENTER);
	fill_rectangle(fb, (rect_t){.x = 0, .y = 0, .width = DISPLAY_WIDTH, .height = 16}, 0xFFFF);
	tf_draw_str(fb, font, "ogo-fm " APP_VERSION, (point_t){.x = 3, .y = 3});
	display_update_rect((rect_t) {.x = 0, .y = 0, .width = DISPLAY_WIDTH, .height = 16});

	file_browser();

	app_shutdown();
}

#ifdef SIM

int main(int argc, char const* argv[])
{
	app_main();
	return 0;
}

#endif