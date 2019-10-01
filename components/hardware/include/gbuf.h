#pragma once

#include <stdbool.h>
#include <stdint.h>

// Graphics buffer, mainly used for framebuffer and graphics
typedef struct {
	uint16_t width;
	uint16_t height;
	uint16_t bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
	bool big_endian;
	uint8_t *data;
} gbuf_t;

gbuf_t *gbuf_new(uint16_t width, uint16_t height, uint16_t bytes_per_pixel, bool big_endian);
void gbuf_free(gbuf_t *g);
