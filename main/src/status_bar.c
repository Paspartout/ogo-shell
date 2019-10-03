#include <display.h>
#include <battery.h>
#include <ui.h>
#include <stdio.h>

#include <status_bar.h>

static void status_bar_draw(void)
{
	// Read battery status
	BatteryInfo info;
	battery_read(&info);

	// Format info into string
	char bat_str[64];
	snprintf(bat_str, 64, "BAT: %d%% %dmV", info.percentage, info.voltage_mv);

	// Draw to fb and update section
	tf_metrics_t m = tf_get_str_metrics(ui_font_black, bat_str);
	rect_t right_half = {.x = 160, .y = 0, .width = 160, .height = 16};
	fill_rectangle(fb, right_half, 0xFFFF);
	tf_draw_str(fb, ui_font_black, bat_str, (point_t){.x = 320 - m.width - 3, .y = 3});
	display_update_rect(right_half);
}

#ifndef SIM

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define UPDATE_PERIOD_SEC 30

void status_bar_task(void *arg)
{
	for (;;) {
		status_bar_draw();
		vTaskDelay((UPDATE_PERIOD_SEC * 1000) / portTICK_PERIOD_MS);
	}
}

void status_bar_start(void) { xTaskCreate(status_bar_task, "status_task", 4096, NULL, 5, NULL); }
#else
void status_bar_start(void) { status_bar_draw(); }
#endif
