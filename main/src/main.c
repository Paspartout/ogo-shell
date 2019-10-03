#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

// Driver/Hardware
#include <backlight.h>
#include <display.h>
#include <event.h>
#include <keypad.h>
#include <sdcard.h>
#include <system.h>
#include <battery.h>
#include <status_bar.h>
#include <settings.h>
#include <audio.h>

#include <OpenSans_Regular_11X12.h>
#include <gbuf.h>
#include <graphics.h>
#include <tf.h>
#include <ui.h>

#include <file_browser.h>

#ifndef SIM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

#ifndef APP_NAME
#define APP_NAME "ogo-shell"
#endif

#define ARRAY_LENGTH(array) (sizeof((array)) / sizeof((array)[0]))

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void ui_display_msg(const char *title, const char *msg)
{
	tf_metrics_t m1 = tf_get_str_metrics(ui_font_red, title);
	tf_metrics_t m2 = tf_get_str_metrics(ui_font_red, msg);
	// determine message location
	point_t text_location = {
	    .x = fb->width / 2 - (MAX(m1.width, m2.width)) / 2,
	    .y = fb->height / 2 - (m1.height + m2.height) / 2,
	};
	rect_t clear_rec = {
	    .x = 0,
	    .y = text_location.y,
	    .width = fb->width,
	    .height = m1.height + m2.height,
	};

	fill_rectangle(fb, clear_rec, 0x0000);
	tf_draw_str(fb, ui_font_red, title, text_location);
	text_location.y += m1.height;
	tf_draw_str(fb, ui_font_red, msg, text_location);

	display_update();
}

static void load_all_settings(void)
{
	int32_t value;
	if (settings_load(SettingAudioVolume, &value) == 0) {
		audio_volume_set(value);
	}
	if (settings_load(SettingAudioOutput, &value) == 0) {
		audio_output_set((AudioOutput)value);
	}
}

static int app_init(void)
{
	display_init();
	backlight_init();
	backlight_percentage_set(50);
	keypad_init();
	event_init();
	system_led_init();
	battery_init();
	ui_init();
	settings_init();
	load_all_settings();

	// Setup sdcard and display error message on failure
	// TODO: Make it nonfatal so user can still browse SPIFFS or so
	if ((sdcard_init("/sdcard")) != 0) {
		ui_display_msg("SDCARD ERROR!", "Please insert the sdcard and restart the device.");
		event_t ev;
		for (;;) {
			wait_event(&ev);
			if (ev.type == EVENT_TYPE_KEYPAD || ev.type == EVENT_TYPE_QUIT)
				break;
			else if (ev.type == EVENT_TYPE_UPDATE) {
				display_update();
			}
		}
		return -1;
	}

	return 0;
}

static void app_shutdown(void)
{
	ui_free();
	sdcard_deinit();
	display_poweroff();
	system_reboot_to_firmware();
}

void app_main_task(void *arg)
{
	if (app_init() != 0) {
		app_shutdown();
		return;
	}

	// Draw title
	fill_rectangle(fb, (rect_t){.x = 0, .y = 0, .width = DISPLAY_WIDTH, .height = 16}, 0xFFFF);
	tf_draw_str(fb, ui_font_black, APP_NAME " " APP_VERSION, (point_t){.x = 3, .y = 3});
	display_update_rect((rect_t){.x = 0, .y = 0, .width = DISPLAY_WIDTH, .height = 16});
	status_bar_start();

	file_browser();

	app_shutdown();
}

void app_main(void)
{
#ifdef SIM
	app_main_task(NULL);
#else
	xTaskCreate(app_main_task, "main_task", 16384 * 2, NULL, 5, NULL);
#endif
}

#ifdef SIM

int main(int argc, char const *argv[])
{
	app_main();
	return 0;
}

#endif
