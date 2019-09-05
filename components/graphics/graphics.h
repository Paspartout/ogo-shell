#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "gbuf.h"
#include "point.h"
#include "rect.h"

typedef enum draw_style_t {
    DRAW_STYLE_SOLID,
    DRAW_STYLE_DOTTED,
} draw_style_t;

void blit(gbuf_t *dst, rect_t dst_rect, gbuf_t *src, rect_t src_rect);
void draw_line(gbuf_t *g, point_t start, point_t end, draw_style_t style, uint16_t color);
void draw_rectangle(gbuf_t *g, rect_t r, enum draw_style_t style, uint16_t color);
void draw_rectangle3d(gbuf_t *g, rect_t r, uint16_t color_nw, uint16_t color_se);
void fill_rectangle(gbuf_t *g, rect_t rect, uint16_t color);
