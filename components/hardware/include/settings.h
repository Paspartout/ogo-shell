#pragma once
#include <stdint.h>
#include <stddef.h>

typedef enum Setting {
	SettingAudioVolume = 0,
	SettingAudioOutput,
	SettingPlayingMode,
	SettingLastPath,
	SettingLastSelection,
	SettingLastScroll,
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

/// Load string setting.
/// Return 0 if saving was sucessfull
int settings_load_str(Setting setting, char *value_out, size_t value_len);

/// Save string setting.
/// Return 0 if saving was sucessfull
int settings_save_str(Setting setting, const char *value);
