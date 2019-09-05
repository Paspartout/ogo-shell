#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t sdcard_init(const char *mount_path);
esp_err_t sdcard_deinit(void);
bool sdcard_present(void);