#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <SDL2/SDL.h>

#include <gbuf.h>
#include <graphics.h>
#include <OpenSans_Regular_11X12.h>
#include <tf.h>

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

// TODO: Move to display_sdl.h
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* canvas = NULL;
gbuf_t *fb;

int display_init() {
	if ( SDL_Init(SDL_INIT_VIDEO) < 0) {
			fprintf(stderr, "error initializing SDL: %s\n", SDL_GetError());
			return -1;
	}
	window = SDL_CreateWindow("ogo-fm", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		fprintf(stderr, "error creating SDL window: %s\n", SDL_GetError());
		return -1;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		fprintf(stderr, "error creating SDL renderer: %s\n", SDL_GetError());
		return -1;
	}
	// TODO: Not sure if this works with RGB565
	canvas = SDL_CreateTexture(renderer, 
			SDL_PIXELFORMAT_RGB565,
			SDL_TEXTUREACCESS_STREAMING,
			SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	fb = gbuf_new(SCREEN_WIDTH, SCREEN_HEIGHT, 2, LITTLE_ENDIAN);
}

void display_deinit() {
	gbuf_free(fb);
	SDL_DestroyTexture(canvas);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void display_clear(uint16_t color) {
	// TODO: Clear canas
}

void display_update(void) {
	// TODO: Copy gbuf to canvas and update screen
	int pitch;
	void *pixels;
	SDL_LockTexture(canvas, NULL, &pixels, &pitch);
	memcpy(pixels, fb->data, fb->width * fb->height * fb->bytes_per_pixel);
	SDL_UnlockTexture(canvas);
	SDL_RenderCopy(renderer, canvas, NULL, NULL);
	SDL_RenderPresent(renderer);
}

int ls(const char* path, tf_t *font) {
	DIR *dir = opendir(path);
	if (dir == NULL) {
		perror("opendir");
		return -1;
	}

	errno = 0;
	int yoff = 0;
	for(struct dirent* entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
		if (errno != 0) {
			perror("readdir");
			break;
		}
		// printf("%c - %s\n", entry->d_type == DT_DIR ? 'd' : 'f', entry->d_name);
		tf_draw_str(fb, font, entry->d_name, (point_t){.x = 10, .y = 20 + yoff});
		yoff += 20;
	}

	closedir(dir);
}

int main(int argc, char const* argv[])
{
	display_init();

	tf_t *font = tf_new(&font_OpenSans_Regular_11X12, 0x0000, 0, TF_ALIGN_CENTER);
	tf_t *font_white = tf_new(&font_OpenSans_Regular_11X12, 0xFFFF, 0, TF_ALIGN_CENTER);
	fill_rectangle(fb, (rect_t){.x = 0, .y = 0, .width = SCREEN_WIDTH, .height = 16}, 0xFFFF);
	tf_draw_str(fb, font, "ogo-fm", (point_t){.x = 40, .y = 3});
	ls(".", font_white);
	display_update();

	bool quit = false;
	while(!quit) {
		// Handle inputs
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = true;
				break;
			}
			display_update();
		}

	}

	display_deinit();
	return 0;
}
