#include <stdint.h>

#include <driver/adc.h>
#include <keypad.h>

#define KEYPAD_IO_X ADC1_CHANNEL_6
#define KEYPAD_IO_Y ADC1_CHANNEL_7
#define KEYPAD_IO_SELECT GPIO_NUM_27
#define KEYPAD_IO_START GPIO_NUM_39
#define KEYPAD_IO_A GPIO_NUM_32
#define KEYPAD_IO_B GPIO_NUM_33
#define KEYPAD_IO_MENU GPIO_NUM_13
#define KEYPAD_IO_VOLUME GPIO_NUM_0

void keypad_init(void)
{
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(KEYPAD_IO_X, ADC_ATTEN_11db);
	adc1_config_channel_atten(KEYPAD_IO_Y, ADC_ATTEN_11db);

	gpio_set_direction(KEYPAD_IO_SELECT, GPIO_MODE_INPUT);
	gpio_set_pull_mode(KEYPAD_IO_SELECT, GPIO_PULLUP_ONLY);

	gpio_set_direction(KEYPAD_IO_START, GPIO_MODE_INPUT);

	gpio_set_direction(KEYPAD_IO_A, GPIO_MODE_INPUT);
	gpio_set_pull_mode(KEYPAD_IO_A, GPIO_PULLUP_ONLY);

	gpio_set_direction(KEYPAD_IO_B, GPIO_MODE_INPUT);
	gpio_set_pull_mode(KEYPAD_IO_B, GPIO_PULLUP_ONLY);

	gpio_set_direction(KEYPAD_IO_MENU, GPIO_MODE_INPUT);
	gpio_set_pull_mode(KEYPAD_IO_MENU, GPIO_PULLUP_ONLY);

	gpio_set_direction(KEYPAD_IO_VOLUME, GPIO_MODE_INPUT);
}

uint16_t keypad_sample(void)
{
	uint16_t sample = 0;

	int joyX = adc1_get_raw(KEYPAD_IO_X);
	int joyY = adc1_get_raw(KEYPAD_IO_Y);

	if (joyX > 2048 + 1024) {
		sample |= KEYPAD_LEFT;
	} else if (joyX > 1024) {
		sample |= KEYPAD_RIGHT;
	}

	if (joyY > 2048 + 1024) {
		sample |= KEYPAD_UP;
	} else if (joyY > 1024) {
		sample |= KEYPAD_DOWN;
	}

	if (!gpio_get_level(KEYPAD_IO_SELECT)) {
		sample |= KEYPAD_SELECT;
	}

	if (!gpio_get_level(KEYPAD_IO_START)) {
		sample |= KEYPAD_START;
	}

	if (!gpio_get_level(KEYPAD_IO_A)) {
		sample |= KEYPAD_A;
	}

	if (!gpio_get_level(KEYPAD_IO_B)) {
		sample |= KEYPAD_B;
	}

	if (!gpio_get_level(KEYPAD_IO_MENU)) {
		sample |= KEYPAD_MENU;
	}

	if (!gpio_get_level(KEYPAD_IO_VOLUME)) {
		sample |= KEYPAD_VOLUME;
	}

	return sample;
}

uint16_t keypad_debounce(uint16_t sample, uint16_t *changes)
{
	static uint16_t state, cnt0, cnt1;
	uint16_t delta, toggle;

	delta = sample ^ state;
	cnt1 = (cnt1 ^ cnt0) & delta;
	cnt0 = ~cnt0 & delta;

	toggle = delta & ~(cnt0 | cnt1);
	state ^= toggle;
	if (changes) {
		*changes = toggle;
	}

	return state;
}
