#include <emulator_launcher.h>

#include <stdio.h>
#include <string.h>
#include <settings.h>
#include <stdlib.h>
#include <limits.h>
#include <ui.h>
#include <assert.h>

#ifndef SIM
#include <nvs_flash.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#endif

static char *get_rom_partition_label(FileType ft)
{
	switch (ft) {
	case FileTypeGB:
	case FileTypeGBC:
		return "gnuboy";
	case FileTypeNES:
		return "nesemu";
	case FileTypeSMS:
	case FileTypeGG:
	case FileTypeCOL:
		return "smsplusgx";
	default:
		return NULL;
	}
}

int emulator_launcher(EmulatorLauncherParam param)
{
	char rom_path[PATH_MAX];
	snprintf(rom_path, PATH_MAX, "%s/%s", param.cwd, param.entry->name);
	const char *emu_part_label = get_rom_partition_label(param.rom_filetype);
	assert(emu_part_label != NULL);

	fprintf(stderr, "Launching partition %s with rom %s\n", emu_part_label, rom_path);

#ifndef SIM
	nvs_handle handle;
	static const char *nvskey_rom_path = "RomFilePath";
	static const char *odroid_namespace = "Odroid";

	// Set rom file to path from entry
	if (nvs_open(odroid_namespace, NVS_READWRITE, &handle) != ESP_OK) {
		ui_message_error("could not open nvs namespace");
		return -1;
	}
	if (nvs_set_str(handle, nvskey_rom_path, rom_path) != ESP_OK) {
		ui_message_error("could not set rom path");
		nvs_close(handle);
		return -1;
	}

	nvs_close(handle);

	// Reboot to emulator if possible
	const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, emu_part_label);
	if (partition == NULL) {
		ui_message_error("no matching emulator partition found");
		return -1;
	}
	if (esp_ota_set_boot_partition(partition) != ESP_OK) {
		ui_message_error("Could not set emulator partition");
		return -1;
	}

	// Save current path to nvs
	settings_save_str(SettingLastPath, param.cwd);
	settings_save(SettingLastSelection, param.fb_selection);
	settings_save(SettingLastScroll, param.fb_scroll);

	esp_restart();
#endif

	return 0;
}
