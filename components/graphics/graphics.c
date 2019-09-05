#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <assert.h>

#include "graphics.h"


void blit(gbuf_t *dst, rect_t dst_rect, gbuf_t *src, rect_t src_rect)
{
    assert(dst_rect.width == src_rect.width);
    assert(dst_rect.height == src_rect.height);
    assert(dst_rect.width >= 0);
    assert(dst_rect.height >= 0);

    /* src_rect.width and src_rect.height are not used after this point */

    if (dst_rect.x < 0) {
        short clip = -dst_rect.x;
        dst_rect.x += clip;
        src_rect.x += clip;
        dst_rect.width -= clip;
    }

    if (dst_rect.y < 0) {
        short clip = -dst_rect.y;
        dst_rect.y += clip;
        src_rect.y += clip;
        dst_rect.height -= clip;
    }

    if (dst_rect.x + dst_rect.width > dst->width) {
        dst_rect.width -= (dst_rect.x + dst_rect.width) - dst->width;
    }

    if (dst_rect.y + dst_rect.height > dst->height) {
        dst_rect.height -= (dst_rect.y + dst_rect.height) - dst->height;
    }

    if (src_rect.x < 0) {
        short clip = -src_rect.x;
        dst_rect.x += clip;
        src_rect.x += clip;
        dst_rect.width -= clip;
    }

    if (src_rect.y < 0) {
        short clip = -src_rect.y;
        dst_rect.y += clip;
        src_rect.y += clip;
        dst_rect.height -= clip;
    }

    if (src_rect.x + dst_rect.width > src->width) {
        dst_rect.width -= (src_rect.x + dst_rect.width) - src->width;
    }

    if (src_rect.y + dst_rect.height > src->height) {
        dst_rect.height -= (src_rect.y + dst_rect.height) - src->height;
    }

    if (src->bytes_per_pixel == 2 && dst->bytes_per_pixel == 2) {
        if (src->big_endian == dst->big_endian) {
            // Simply copy memory line wise
            for (short yoff = 0; yoff < dst_rect.height; yoff++) {
                uint16_t *dst_addr = ((uint16_t *)dst->data) + (dst_rect.y + yoff) * dst->width + dst_rect.x;
                uint16_t *src_addr = ((uint16_t *)src->data) + (src_rect.y + yoff) * src->width + src_rect.x;
                memcpy(dst_addr, src_addr, dst_rect.width * dst->bytes_per_pixel);
            }
        } else {
            // Copy memory and do byte swap
            for (short yoff = 0; yoff < dst_rect.height; yoff++) {
                uint16_t *dst_addr = ((uint16_t *)dst->data) + (dst_rect.y + yoff) * dst->width + dst_rect.x;
                uint16_t *src_addr = ((uint16_t *)src->data) + (src_rect.y + yoff) * src->width + src_rect.x;
                for (short xoff = 0; xoff < dst_rect.width; xoff++) {
                    *(dst_addr + xoff) = *(src_addr + xoff) << 8 | *(src_addr + xoff) >> 8;
                }
            }
        }
    }
}

void draw_line(gbuf_t *g, point_t start, point_t end, draw_style_t style, uint16_t color)
{
    short inc, dx, dy;

    if (g->big_endian) {
        color = color << 8 | color >> 8;
    }

    dx = abs(end.x - start.x);
    dy = abs(end.y - start.y);
    if (dx >= dy) {
        inc = dx;
    } else {
        inc = dy;
    }
    dx /= inc;
    dy /= inc;

    if (style == DRAW_STYLE_DOTTED) {
        for (int i = 0; i <= inc; i += 2) {
            uint16_t *pixel = ((uint16_t *)g->data) + (start.y * g->width) + start.x;
            *pixel = color;
            start.x += dx*2;
            start.y += dy*2;
        }
    } else {
        for (int i = 0; i <= inc; i++) {
            uint16_t *pixel = ((uint16_t *)g->data) + (start.y * g->width) + start.x;
            *pixel = color;
            start.x += dx;
            start.y += dy;
        }
    }
}

void draw_rectangle(gbuf_t *g, rect_t r, enum draw_style_t style, uint16_t color)
{
    point_t start, end;

    start.x = r.x;
    start.y = r.y;
    end.x = r.x + r.width - 1;
    end.y = r.y;
    draw_line(g, start, end, style, color);

    start.x = r.x;
    start.y = r.y;
    end.x = r.x;
    end.y = r.y + r.height - 1;
    draw_line(g, start, end, style, color);

    start.x = r.x;
    start.y = r.y + r.height - 1;
    end.x = r.x + r.width - 1;
    end.y = r.y + r.height - 1;
    draw_line(g, start, end, style, color);

    start.x = r.x + r.width - 1;
    start.y = r.y;
    end.x = r.x + r.width - 1;
    end.y = r.y + r.height - 1;
    draw_line(g, start, end, style, color);
}

void draw_rectangle3d(gbuf_t *g, rect_t r, uint16_t color_nw, uint16_t color_se)
{
    point_t start, end;

    start.x = r.x;
    start.y = r.y;
    end.x = r.x + r.width - 1;
    end.y = r.y;
    draw_line(g, start, end, DRAW_STYLE_SOLID, color_nw);

    start.x = r.x;
    start.y = r.y;
    end.x = r.x;
    end.y = r.y + r.height - 1;
    draw_line(g, start, end, DRAW_STYLE_SOLID, color_nw);

    start.x = r.x;
    start.y = r.y + r.height - 1;
    end.x = r.x + r.width - 1;
    end.y = r.y + r.height - 1;
    draw_line(g, start, end, DRAW_STYLE_SOLID, color_se);

    start.x = r.x + r.width - 1;
    start.y = r.y;
    end.x = r.x + r.width - 1;
    end.y = r.y + r.height - 1;
    draw_line(g, start, end, DRAW_STYLE_SOLID, color_se);
}

void fill_rectangle(gbuf_t *g, rect_t rect, uint16_t color)
{
    if (g->big_endian) {
        color = color << 8 | color >> 8;
    }

    for (short yoff = 0; yoff < rect.height; yoff++) {
        uint16_t *addr  = ((uint16_t *)g->data) + (rect.y + yoff) * g->width + rect.x;
        for (short xoff = 0; xoff < rect.width; xoff++) {
            *(addr + xoff) = color;
        }
    }
}
