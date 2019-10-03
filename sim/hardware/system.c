#include <stdio.h>
#include <system.h>

void system_reboot_to_firmware(void) { fprintf(stderr, "Rebooting to firmware...\n"); }
void system_led_init(void) {}
void system_led_set(bool value) { printf("Turning LED %s\n", value ? "On" : "Off"); }
void system_enter_deep_sleep(void) { printf("Going to deep sleep...\n"); }
