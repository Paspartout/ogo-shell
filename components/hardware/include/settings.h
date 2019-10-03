#pragma once
#include <stdint.h>

typedef enum Setting {
	SettingAudioVolume = 0,
	SettingAudioOutput,
	SettingPlaylistMode,
	SettingMax,
} Setting;

/// Initalize settings
int settings_init(void);

/// Initalize settings
void settings_deinit(void);

/// Load setting.
/// Return 0 if loaded, false if not found
int settings_load(Setting setting, int32_t *value_out);

/// Save setting.
/// Return 0 if saving was sucessfull
int settings_save(Setting setting, int32_t value);
