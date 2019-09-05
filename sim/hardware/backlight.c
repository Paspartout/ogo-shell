#include <stdio.h>
#include <stdbool.h>

#include <backlight.h>

static bool initialized = false;

void backlight_init(void) {
	fprintf(stderr, "Initializing backlight...\n");
	initialized = true;
}

void backlight_percentage_set(int value) {
	fprintf(stderr, "Setting backlight to %d\n", value);
}
int is_backlight_initialized(void) {
	return initialized;
}