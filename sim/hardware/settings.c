#include <settings.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static char *settings_keys[SettingMax] = {
    "volume", "output", "playmode", "last_path", "last_selection",
};

typedef enum KeyType {
	TypeInt,
	TypeStr,
} KeyType;

static KeyType settings_types[SettingMax] = {
    TypeInt, TypeInt, TypeInt, TypeStr, TypeInt,
};

#define PATH_LEN 1024

static struct {
	int32_t volume;
	int32_t output;
	int32_t playmode;
	char last_path[PATH_LEN];
	int32_t last_selection;
	int32_t last_scroll;
} settings;

#define SETTINGS_FILE "settings.bin"

/// Initalize settings
int settings_init(void)
{
	memset(&settings, 0, sizeof(settings));

	// Create settings file
	FILE *fp = fopen(SETTINGS_FILE, "w");
	assert(fp != NULL);
	long size = ftell(fp);
	assert(fseek(fp, 0, SEEK_END) == 0);
	fprintf(stderr, "size: %ld\n", size);

	// Read settings from file if its long enough
	if (size <= (long)sizeof(settings)) {
		fprintf(stderr, "reading %ld bytes from file into %lu\n", size, sizeof(settings));
		fread(&settings, (size_t)size, 1, fp);
	}

	fclose(fp);
	return 0;
}

/// Initalize settings
void settings_deinit(void)
{
	// TODO: Simulate
}

/// Load setting.
/// Return 0 if loaded, false if not found
int settings_load(Setting setting, int32_t *value_out)
{
	assert(setting < SettingMax && setting >= 0);
	fprintf(stderr, "Loading %s\n", settings_keys[setting]);
	switch (setting) {
	case SettingAudioVolume:
		*value_out = settings.volume;
		break;
	case SettingAudioOutput:
		*value_out = settings.output;
		break;
	case SettingPlayingMode:
		*value_out = settings.playmode;
		break;
	case SettingLastSelection:
		*value_out = settings.last_selection;
		break;
	case SettingLastScroll:
		*value_out = settings.last_scroll;
		break;
	default:
		return -1;
	}
	return 0;
}

/// Save setting.
/// Return 0 if saving was sucessfull
int settings_save(Setting setting, int32_t value)
{
	assert(setting < SettingMax && setting >= 0);
	fprintf(stderr, "Saving %s as %d\n", settings_keys[setting], value);
	switch (setting) {
	case SettingAudioVolume:
		settings.volume = value;
		break;
	case SettingAudioOutput:
		settings.output = value;
		break;
	case SettingPlayingMode:
		settings.playmode = value;
		break;
	case SettingLastSelection:
		settings.last_selection = value;
		break;
	case SettingLastScroll:
		settings.last_scroll = value;
		break;
	default:
		return -1;
	}
	return 0;
}

/// Load string setting.
/// Return 0 if saving was sucessfull
int settings_load_str(Setting setting, char *value_out, size_t value_len)
{
	assert(setting < SettingMax && setting >= 0);
	assert(settings_types[setting] == TypeStr);
	fprintf(stderr, "Loading %s\n", settings_keys[setting]);
	switch (setting) {
	case SettingLastPath:
		strncpy(value_out, settings.last_path, value_len);
		break;
	default:
		return -1;
	}

	return -1;
}

/// Save string setting.
/// Return 0 if saving was sucessfull
int settings_save_str(Setting setting, const char *value)
{
	assert(setting < SettingMax && setting >= 0);
	assert(settings_types[setting] == TypeStr);
	fprintf(stderr, "Saving %s as %s\n", settings_keys[setting], value);

	switch (setting) {
	case SettingLastPath:
		strncpy(settings.last_path, value, PATH_LEN);
		break;
	default:
		return -1;
	}

	return 0;
}
