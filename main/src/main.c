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
#include <sdcard.h>

#include <gbuf.h>
#include <graphics.h>
#include <OpenSans_Regular_11X12.h>
#include <tf.h>

#include <file_browser.h>

#ifndef SIM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif


#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

#ifndef APP_NAME
#define APP_NAME "ogo-fm"
#endif

#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

static tf_t *ui_font;
static tf_t *ui_font_error;

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void ui_display_msg(const char *title, const char *msg) {
	tf_metrics_t m1 = tf_get_str_metrics(ui_font_error, title);
	tf_metrics_t m2 = tf_get_str_metrics(ui_font_error, msg);
	// determine message location
	point_t text_location = {
		.x = fb->width/2 - (MAX(m1.width,m2.width))/2,
		.y = fb->height/2 - (m1.height+m2.height)/2,
	};
	rect_t clear_rec = {
		.x = 0,
		.y = text_location.y,
		.width = fb->width,
		.height = m1.height+m2.height,
	};

	fill_rectangle(fb, clear_rec, 0x0000);
	tf_draw_str(fb, ui_font_error, title, text_location);
	text_location.y += m1.height;
	tf_draw_str(fb, ui_font_error, msg, text_location);

	display_update();
}

static int app_init(void) {
	display_init();
    backlight_init();
    keypad_init();
	event_init();

	ui_font = tf_new(&font_OpenSans_Regular_11X12, 0x0000, 0, TF_ALIGN_CENTER);
	ui_font_error = tf_new(&font_OpenSans_Regular_11X12, 0xF800, 0, TF_ALIGN_CENTER);

	// Setup sdcard and display error message on failure
	// TODO: Make it nonfatal so user can still browse SPIFFS or so
	if((sdcard_init("/sdcard")) != 0) {
		ui_display_msg("SDCARD ERROR!", "Please insert the sdcard and restart the device.");
		event_t ev;
		for(;;) {
			wait_event(&ev);
			if (ev.type == EVENT_TYPE_KEYPAD || ev.type == EVENT_TYPE_QUIT)
				break;
			else if(ev.type == EVENT_TYPE_UPDATE) {
				display_update();
			}
		}
		return -1;
	}

	return 0;
}

static void app_shutdown(void) {
	tf_free(ui_font);
	tf_free(ui_font_error);
	sdcard_deinit();
	display_poweroff();
	reboot_to_firmware();
}

void app_main_task(void *arg) {
	if (app_init() != 0) {
		app_shutdown();
		return;
	}

	// Draw title
	fill_rectangle(fb, (rect_t){.x = 0, .y = 0, .width = DISPLAY_WIDTH, .height = 16}, 0xFFFF);
	tf_draw_str(fb, ui_font, APP_NAME " " APP_VERSION, (point_t){.x = 3, .y = 3});
	display_update_rect((rect_t) {.x = 0, .y = 0, .width = DISPLAY_WIDTH, .height = 16});

	file_browser();

	app_shutdown();
}

void
app_main(void)
{
#ifdef SIM
	app_main_task(NULL);
#else
    xTaskCreate(app_main_task, "main_task", 16384*2, NULL, 5, NULL);
#endif
}

#ifdef SIM

int main(int argc, char const* argv[])
{
	app_main();
	return 0;
}

#endif