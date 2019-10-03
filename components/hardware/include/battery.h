#pragma once

#include <stdint.h>

typedef struct BatteryInfo {
	/// Battery voltage in millivolts, should be between 3500 and 4200ish
	uint16_t voltage_mv;
	/// Percentage of charge. Should be between 0-100.
	uint8_t percentage;
} BatteryInfo;

/// Initialize battery measurement hardware.
int battery_init(void);
int battery_deinit(void);
int battery_read(BatteryInfo *info);
