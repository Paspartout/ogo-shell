#include <nvs_flash.h>
#include <nvs.h>
#include <assert.h>

#include <settings.h>

static nvs_handle handle;

static char *settings_keys[SettingMax] = {
    "volume",
    "output",
    "playmode",
};

/// Initalize settings
int settings_init(void)
{
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		if (nvs_flash_erase() != ESP_OK) {
			return -1;
		}
		err = nvs_flash_init();
	}
	if (err != ESP_OK) {
		return -1;
	}

	if (nvs_open("ogo-shell", NVS_READWRITE, &handle) != ESP_OK) {
		return -1;
	}

	return 0;
}

/// Initalize settings
void settings_deinit(void)
{
	nvs_close(handle);
	nvs_flash_deinit();
}

/// Load setting.
/// Return 0 if loaded, false if not found
int settings_load(Setting setting, int32_t *value_out)
{
	assert(setting < SettingMax && setting >= 0);
	return nvs_get_i32(handle, settings_keys[setting], value_out);
}

/// Save setting.
/// Return 0 if saving was sucessfull
int settings_save(Setting setting, int32_t value)
{
	assert(setting < SettingMax && setting >= 0);
	return nvs_set_i32(handle, settings_keys[setting], value);
}
