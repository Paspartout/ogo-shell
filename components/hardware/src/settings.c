#include <nvs_flash.h>
#include <nvs.h>
#include <assert.h>

#include <settings.h>

static nvs_handle handle;

typedef enum KeyType {
	TypeInt,
	TypeStr,
} KeyType;

static KeyType settings_types[SettingMax] = {
    TypeInt, TypeInt, TypeInt, TypeStr, TypeInt, TypeInt,
};

static char *settings_keys[SettingMax] = {
    "volume", "output", "playmode", "last_path", "last_selection", "last_scrolling",
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
	assert(settings_types[setting] == TypeInt);
	return nvs_get_i32(handle, settings_keys[setting], value_out);
}

/// Save setting.
/// Return 0 if saving was sucessfull
int settings_save(Setting setting, int32_t value)
{
	assert(setting < SettingMax && setting >= 0);
	assert(settings_types[setting] == TypeInt);
	return nvs_set_i32(handle, settings_keys[setting], value);
}

/// Load string setting.
/// Return 0 if saving was sucessfull
int settings_load_str(Setting setting, char *value_out, size_t value_len)
{
	size_t len;
	assert(setting < SettingMax && setting >= 0);
	assert(settings_types[setting] == TypeStr);
	nvs_get_str(handle, settings_keys[setting], NULL, &len);
	if (len > value_len) {
		return -1;
	}
	return nvs_get_str(handle, settings_keys[setting], value_out, &len);
}

/// Save string setting.
/// Return 0 if saving was sucessfull
int settings_save_str(Setting setting, const char *value)
{
	assert(setting < SettingMax && setting >= 0);
	assert(settings_types[setting] == TypeStr);
	return nvs_set_str(handle, settings_keys[setting], value);
}
