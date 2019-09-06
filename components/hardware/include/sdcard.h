#pragma once

#include <stdbool.h>

int sdcard_init(const char *mount_path);
int sdcard_deinit(void);
bool sdcard_present(void);