#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tf.h"


typedef struct {
    const char *s;
    size_t len;
    short width;
    bool ellipsis;
} tf_iterinfo_t;


tf_t *tf_new(const struct tf_font_t *font, uint16_t color, short width, uint32_t flags)
{
    tf_t *tf = calloc(1, sizeof(tf_t));
    assert(tf != NULL);

    tf->font = font;
    tf->color = color;
    tf->width = width;
    tf->flags = flags;
    return tf;
}

void tf_free(tf_t *tf)
{
    free(tf);
}

static tf_iterinfo_t tf_iter_lines(tf_t *tf, const char *start)
{
    static const char *s = NULL;
    tf_iterinfo_t ii = { 0 };
    short width = 0;
    short ellipsis_width = 0;

    if (start) {
        s = start;
    } else {    
        if (!s) {
            return ii;
        }
        while (*s == ' ') {
            s++;
        }
    }

    if (tf->flags & TF_ELIDE && '.' >= tf->font->first && '.' <= tf->font->last) {
        ellipsis_width = (tf->font->widths ? tf->font->widths['.' - tf->font->first] : tf->font->width) * 3;
    }

    const char *p = s;
    while (*p) {
        if (*p < tf->font->first || *p > tf->font->last) {
            p++;
            continue;
        }

        short char_width = tf->font->widths ? tf->font->widths[*p - tf->font->first] : tf->font->width;

        if ((tf->flags & TF_WORDWRAP || tf->flags & TF_ELIDE) && tf->width > 0 && width + char_width > tf->width) {
            const char *q = p;
            short sub = 0;
            while (p--) {
                if (*p < tf->font->first || *p > tf->font->last) {
                    continue;
                }
                sub += tf->font->widths ? tf->font->widths[*p - tf->font->first] : tf->font->width;
                if (tf->flags & TF_ELIDE) {
                    if (width - sub + ellipsis_width <= tf->width) {
                        width = width - sub + ellipsis_width;
                        ii.s = s;
                        ii.len = p - s;
                        ii.width = width;
                        ii.ellipsis = true;
                        s = "";
                        return ii;
                    }
                } else if (*p == ' ') {
                    break;
                }
                if (width <= sub) {
                    p = q;
                    sub = 0;
                    break;
                }
            }
            width -= sub;    
            break;
        }
        width += char_width;
        p++;
    }

    ii.s = s;
    ii.len = p - s;
    ii.width = width;
    s = p;

    return ii;
}

tf_metrics_t tf_get_str_metrics(tf_t *tf, const char *s)
{
    tf_metrics_t m = { 0 };
    tf_iterinfo_t ii = tf_iter_lines(tf, s);

    while (ii.len) {
        m.height += tf->font->height;
        /* get maximum line width */
        m.width = ii.width > m.width ? ii.width : m.width;

        ii = tf_iter_lines(tf, NULL);
    }

    return m;
}

short tf_draw_glyph(gbuf_t *g, tf_t *tf, char c, point_t p)
{
    if (c < tf->font->first || c > tf->font->last) {
        c = '?';
    }

    short width = tf->font->widths ? tf->font->widths[c - tf->font->first] : tf->font->width;

    short xstart = p.x < 0 ? -p.x : 0;
    short xend = p.x + width > g->width ? g->width - p.x : width;
    short ystart = p.y < 0 ? -p.y : 0;
    short yend = p.y + tf->font->height > g->height ? g->height - p.y : tf->font->height;

    if (tf->clip.width > 0) {
        if (p.x + xstart < tf->clip.x) {
            xstart = tf->clip.x - p.x;
        }
        if (p.x + xend > tf->clip.x + tf->clip.width) {
            xend = tf->clip.x + tf->clip.width - p.x;
        }
    }

    if (tf->clip.height > 0) {
        if (p.y + ystart < tf->clip.y) {
            ystart = tf->clip.y - p.y;
        }
        if (p.y + yend > tf->clip.y + tf->clip.height) {
            yend = tf->clip.y + tf->clip.height - p.y;
        }
    }

    uint16_t color = tf->color;
    if (g->big_endian) {
        color = color << 8 | color >> 8;
    }

    const unsigned char *glyph = tf->font->p + ((tf->font->width + 7) / 8) * tf->font->height * (c - tf->font->first);

    for (short yoff = ystart; yoff < yend; yoff++) {
        uint16_t *pixel = ((uint16_t *)g->data) + (p.y + yoff) * g->width + p.x;
        for (short xoff = xstart; xoff < xend; xoff++) {
            if (glyph[yoff * ((tf->font->width + 7) / 8) + (xoff / 8)] & (1 << (xoff % 8))) {
                *(pixel + xoff) = color;
            }
        }
    }

    return width;
}

void tf_draw_str(gbuf_t *g, tf_t *tf, const char *s, point_t p)
{
    short xoff = 0;
    short yoff = 0;

    tf_iterinfo_t ii = tf_iter_lines(tf, s);
    int line = 1;
    while (true) {
        short ystart = p.y + yoff < 0 ? -(p.y + yoff) : yoff;
        short yend = p.y + yoff + tf->font->height > g->height ? g->height - p.y : yoff + tf->font->height;

        /* TODO: Y clipping */

        if (ystart >= line * tf->font->height || yend <= 0) {
            break;
        }

        if (tf->width <= 0 || !(tf->flags & TF_ALIGN_RIGHT || tf->flags & TF_ALIGN_CENTER)) {
            xoff = 0;
        } else if (tf->flags & TF_ALIGN_RIGHT) {
            xoff = tf->width - ii.width; 
        } else if (tf->flags & TF_ALIGN_CENTER) {
            xoff = (tf->width - ii.width) / 2;
        }

        for (int i = 0; i < ii.len; i++) {
            point_t gp = {p.x + xoff, p.y + yoff};
            xoff += tf_draw_glyph(g, tf, ii.s[i], gp);
            if (tf->clip.width > 0 && xoff + p.x > tf->clip.x + tf->clip.width) {
                break;
            }
        }

        if (ii.ellipsis) {
            point_t gp = {p.x + xoff, p.y + yoff};
            xoff += tf_draw_glyph(g, tf, '.', gp);
            gp.x = p.x + xoff;
            xoff += tf_draw_glyph(g, tf, '.', gp);
            gp.x = p.x + xoff;
            xoff += tf_draw_glyph(g, tf, '.', gp);
        }

        ii = tf_iter_lines(tf, NULL);
        if (ii.len == 0) {
            break;
        }

        yoff += tf->font->height;
        line++;
    }
}
