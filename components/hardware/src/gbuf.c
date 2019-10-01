#include <gbuf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

gbuf_t *gbuf_new(uint16_t width, uint16_t height, uint16_t bytes_per_pixel, bool big_endian)
{
	gbuf_t *g = calloc(1, sizeof(gbuf_t));
	if (g == NULL)
		return NULL;
	g->data = calloc(1, width * height * bytes_per_pixel);
	if (g->data == NULL) {
		free(g);
		return NULL;
	}

	g->width = width;
	g->height = height;
	g->bytes_per_pixel = bytes_per_pixel;
	g->big_endian = big_endian;

	return g;
}

void gbuf_free(gbuf_t *g)
{
	free(g->data);
	free(g);
}
