#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
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
	if ((sdcard_init("/sd")) != 0) {
		ui_message_error("SDCARD ERROR: Please insert the sdcard and restart the device.");
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

#ifdef SIM
static char start_dir_buf[PATH_MAX];
static char *start_dir = start_dir_buf;
#else
#define start_dir "/sd/"
#endif

void app_main_task(void *arg)
{
	if (app_init() != 0) {
		app_shutdown();
		return;
	}

	status_bar_draw();
	status_bar_start();

	file_browser((FileBrowserParam) {.cwd = start_dir});

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
	if (argc > 1) {
		if (realpath(argv[1], (char*)&start_dir_buf) == NULL) {
			perror("Could not resolve start path");
			start_dir = start_dir_buf;
			return -1;
		}
	} else {
		start_dir = "/home/";
	}

	app_main();
	return 0;
}

#endif
