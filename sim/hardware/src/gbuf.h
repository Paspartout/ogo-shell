#pragma once

#include <stdint.h>


typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */  
    uint16_t endian;
    uint8_t data[];
} gbuf_t;


gbuf_t *gbuf_new(uint16_t width, uint16_t height, uint16_t bytes_per_pixel, uint16_t endian);
void gbuf_free(gbuf_t *g);
