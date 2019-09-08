#pragma once

#include <stdint.h>

enum { KEYPAD_UP = 1,
       KEYPAD_RIGHT = 2,
       KEYPAD_DOWN = 4,
       KEYPAD_LEFT = 8,
       KEYPAD_SELECT = 16,
       KEYPAD_START = 32,
       KEYPAD_A = 64,
       KEYPAD_B = 128,
       KEYPAD_MENU = 256,
       KEYPAD_VOLUME = 512,
};

void keypad_init(void);
uint16_t keypad_sample(void);
uint16_t keypad_debounce(uint16_t sample, uint16_t *changes);
