#include <battery.h>

int battery_init(void) { return 0; }
int battery_deinit(void) { return 0; }
int battery_read(BatteryInfo *info)
{
	info->percentage = 100;
	info->voltage_mv = 4200;
	return 0;
}
