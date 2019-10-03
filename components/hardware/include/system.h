#pragma once

#include <stdbool.h>

/// Reboot ESP/Odroid to first found factory or ota partition
void system_reboot_to_firmware(void);
/// Initialize led gpios
void system_led_init(void);
/// Turn led on or off.
void system_led_set(bool on);
/// Turn led on or off.
void system_enter_deep_sleep(void);
