#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>

/** Reboot ESP/Odroid to first found factory or ota partition */
void reboot_to_firmware(void)
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
