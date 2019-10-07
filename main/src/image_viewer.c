#include <image_viewer.h>
#include <display.h>
#include <event.h>
#include <keypad.h>
#include <file_ops.h>
#include <ui.h>
#include <status_bar.h>

#include <stdbool.h>

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_ONLY_GIF

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static struct ViewerState {
	/// Index of currently viewed file/photo in params.entries
	int index;

	bool fullscreen_enabled;
	ImageViewerParams params;
} state;

static const int CANVAS_WIDTH = (DISPLAY_WIDTH);
static const int CANVAS_HEIGHT = (DISPLAY_HEIGHT - 32);

// TODO: Figure this out properly
static float calc_scalefactor(int width, int height, int target_width, int target_height)
{
	float sf = 0.0;
	if (height > width) {
		// Fit height
		sf = target_height / (float)height;
	} else {
		// Fit width
		sf = target_width / (float)width;
	}

	if (sf * height > target_height) {
		sf = target_height / (float)height;
	}
	return sf;
}

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t rgb565 = ((r & (uint32_t)0xF8) << 8) | ((g & (uint32_t)0xFC) << 3) | ((b & (uint32_t)0xF8) >> 3);
	return (uint16_t)rgb565;
}

#define ERROR_BUF_SIZE 1024
static char error_buf[ERROR_BUF_SIZE];

static void draw_pathbar(const char *filename, int width, int height)
{
	assert(filename != NULL);
	char *size_str = NULL;
	char size_buf[32];

	if (width > 0 && height > 0) {
		snprintf(size_buf, 32, "%dx%d", width, height);
		size_str = size_buf;
	}

	ui_draw_pathbar(filename, size_str, false);
}

static int draw_image(const char *img_fullpath, const char *filename)
{
	draw_pathbar("Opening image...", 0, 0);
	// Open Image using stb_image
	int img_width, img_height, n;
	unsigned char *img_data = stbi_load(img_fullpath, &img_width, &img_height, &n, 3);
	if (img_data == NULL) {
		const char *reason = stbi_failure_reason();
		snprintf(error_buf, ERROR_BUF_SIZE, "error: %s", reason);
		return -1;
	}

	// Scaling the image down to fit height or width of canvas/screen
	int target_width, target_height;
	if (state.fullscreen_enabled) {
		target_width = DISPLAY_WIDTH;
		target_height = DISPLAY_HEIGHT;
	} else {
		target_width = CANVAS_WIDTH;
		target_height = CANVAS_HEIGHT;
	}
	const float scale_factor = calc_scalefactor(img_width, img_height, target_width, target_height);
	const int scaled_width = (int)(scale_factor * img_width);
	const int scaled_height = (int)(scale_factor * img_height);

	assert(scaled_width >= 0 && scaled_width <= DISPLAY_WIDTH);
	assert(scaled_height >= 0 && scaled_height <= DISPLAY_HEIGHT);

	gbuf_t *scaled_img = gbuf_new((uint16_t)scaled_width, (uint16_t)scaled_height, 2, false);
	if (scaled_img == NULL) {
		strncpy(error_buf, "error: out of memory", ERROR_BUF_SIZE);
		stbi_image_free(img_data);
		return -1;
	}
	uint16_t *pixels = (uint16_t *)scaled_img->data;
	const float scale_incr = 1 / scale_factor;
	float src_x = 0, src_y = 0;
	for (int y = 0; y < scaled_height; y++) {
		int src_y_i = (int)src_y;
		src_x = 0;
		for (int x = 0; x < scaled_width; x++) {
			// Convert to RGB565 for displaying
			const int pos = (src_y_i * img_width + (int)src_x) * 3;
			uint8_t r = img_data[pos];
			uint8_t g = img_data[pos + 1];
			uint8_t b = img_data[pos + 2];
			pixels[y * scaled_width + x] = rgb565(r, g, b);
			src_x += scale_incr;
		}
		src_y += scale_incr;
	}
	stbi_image_free(img_data);

	rect_t dest_rect = {
	    .x = (short)target_width / 2 - (short)scaled_width / 2,
	    .y = ((short)target_height / 2 - (short)scaled_height / 2) + (state.fullscreen_enabled ? 0 : 32),
	    .width = (short)scaled_width,
	    .height = (short)scaled_height,
	};

	if (!state.fullscreen_enabled) {
		fill_rectangle(fb, (rect_t){.x = 0, .y = 32, .width = (short)target_width, .height = (short)target_height}, 0x0000);
		draw_pathbar(filename, img_width, img_height);
	} else {
		fill_rectangle(fb, (rect_t){.x = 0, .y = 0, .width = (short)target_width, .height = (short)target_height}, 0x0000);
	}

	blit(fb, dest_rect, scaled_img, (rect_t){.x = 0, .y = 0, .width = (short)scaled_width, .height = (short)scaled_height});
	display_update();

	gbuf_free(scaled_img);
	return 0;
}

static char img_pathbuf[PATH_MAX];
static char *get_imgpath(void)
{
	snprintf(img_pathbuf, PATH_MAX, "%s/%s", state.params.cwd, state.params.entries[state.index].name);
	return img_pathbuf;
}

static bool is_image_type(FileType ft) { return ft == FileTypeJPEG || ft == FileTypePNG || ft == FileTypeBMP || ft == FileTypeGIF; }

static void next_prev_img(int diff)
{
	int ret = -1;
	do {
		int new_index = state.index + diff;
		if (new_index > state.params.n_entries - 1) {
			new_index = 0;
		} else if (new_index < 0) {
			new_index = state.params.n_entries - 1;
		}
		state.index = new_index;

		if (is_image_type(fops_determine_filetype(&state.params.entries[state.index]))) {
			ret = draw_image(get_imgpath(), state.params.entries[state.index].name);
		}
	} while (ret != 0);
}

static void exit_fullscreen(void)
{
	const rect_t black_rect = {.x = 0, .y = 31, .height = 1, .width = DISPLAY_WIDTH};
	fill_rectangle(fb, black_rect, 0x0000);
	display_update_rect(black_rect);
	status_bar_draw();
}

static void handle_keypress(uint16_t keys, bool *quit)
{
	switch (keys) {
	case KEYPAD_A:
		break;
	case KEYPAD_B:
		*quit = true;
		break;
	case KEYPAD_UP:
		break;
	case KEYPAD_DOWN:
		break;
	case KEYPAD_RIGHT:
		// Open next image in folder/file list
		next_prev_img(1);
		break;
	case KEYPAD_LEFT:
		// Open previous image in folder/file list
		next_prev_img(-1);
		break;
	case KEYPAD_VOLUME:
		break;
	case KEYPAD_START:
		state.fullscreen_enabled = !state.fullscreen_enabled;
		if (!state.fullscreen_enabled) {
			exit_fullscreen();
		}
		draw_image(get_imgpath(), state.params.entries[state.index].name);
		break;
	case KEYPAD_SELECT:
		break;
	case KEYPAD_MENU:
		*quit = true;
		break;
	}
}

int image_viewer(ImageViewerParams params)
{
	state.params = params;
	state.index = params.index;

	if (draw_image(get_imgpath(), params.entries[state.index].name) != 0) {
		ui_message_error(error_buf);
		return -1;
	}

	bool quit = false;
	event_t event;
	while (!quit) {
		// Handle inputs
		if (wait_event(&event) < 0) {
			continue;
		}
		switch (event.type) {
		case EVENT_TYPE_KEYPAD:
			handle_keypress(event.keypad.pressed, &quit);
			break;
		case EVENT_TYPE_QUIT:
			quit = true;
			break;
		case EVENT_TYPE_UPDATE:
			display_update();
			break;
		default:
			break;
		}
	}

	exit_fullscreen();

	return 0;
}
