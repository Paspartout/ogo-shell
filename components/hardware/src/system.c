#include <system.h>
#include <display.h>
#include <backlight.h>

#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>
#include <driver/gpio.h>

#define LED_PIN GPIO_NUM_2

void system_reboot_to_firmware(void)
{
	const esp_partition_t *part;
	part = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
	// If no factory partition found, use first ota one
	if (part == NULL)
		part = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);

	if (part != NULL) {
		esp_ota_set_boot_partition(part);
	}
	esp_restart();
}

void system_led_init(void)
{
	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
	gpio_set_level(LED_PIN, 0);
}

void system_led_set(bool value) { gpio_set_level(LED_PIN, value ? 1 : 0); }

void system_enter_deep_sleep(void)
{
	backlight_percentage_set(0);
	display_poweroff();
	esp_deep_sleep_start();
}
