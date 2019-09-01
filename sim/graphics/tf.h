#pragma once

#include "graphics.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

enum tf_flags_t {
    TF_ALIGN_RIGHT = 1,
    TF_ALIGN_CENTER = 2,
    TF_WORDWRAP = 4,
    TF_ELIDE = 8,
};

typedef struct tf_font_t tf_font_t;

typedef struct {
    const tf_font_t *font;
    uint16_t color;
    short width;
    uint16_t flags;
    rect_t clip;
} tf_t;

struct tf_font_t {
   const unsigned char *p;
   short width;
   short height;
   char first;
   char last;
   const short *widths;
};

typedef struct  {
    short width;
    short height;
} tf_metrics_t;

tf_t *tf_new(const tf_font_t *font, uint16_t color, short width, uint32_t flags);
void tf_free(tf_t *tf);
tf_metrics_t tf_get_str_metrics(tf_t *tf, const char *s);
short tf_draw_glyph(gbuf_t *g, tf_t *tf, char c, point_t p);
void tf_draw_str(gbuf_t *g, tf_t *tf, const char *s, point_t p);
