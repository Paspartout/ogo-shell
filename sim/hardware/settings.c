#include <settings.h>
#include <stdio.h>
#include <assert.h>

static char *settings_keys[SettingMax] = {
    "volume",
    "output",
    "playmode",
};

/// Initalize settings
int settings_init(void)
{
	// TODO: Simulate
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
	// TODO: Simulate
	return -1;
}

/// Save setting.
/// Return 0 if saving was sucessfull
int settings_save(Setting setting, int32_t value)
{
	assert(setting < SettingMax && setting >= 0);
	fprintf(stderr, "Saving %s as %d\n", settings_keys[setting], value);
	// TODO: Simulate
	return -1;
}
