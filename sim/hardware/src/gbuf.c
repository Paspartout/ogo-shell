#include <stdlib.h>

#include "gbuf.h"

gbuf_t *gbuf_new(uint16_t width, uint16_t height, uint16_t bytes_per_pixel, uint16_t endian)
{
    gbuf_t *g = malloc(sizeof(gbuf_t) + width * height * bytes_per_pixel);
    if (!g) abort();

    g->width = width;
    g->height = height;
    g->bytes_per_pixel = bytes_per_pixel;
    g->endian = endian;

    return g;
}

void gbuf_free(gbuf_t *g)
{
    free(g);
}
