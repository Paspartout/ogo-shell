#include <ui.h>

#include <display.h>
#include <gbuf.h>
#include <tf.h>
#include <graphics.h>
#include <event.h>
#include <OpenSans_Regular_11X12.h>
#include <str_utils.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

tf_t *ui_font_black;
tf_t *ui_font_white;
tf_t *ui_font_red;

void ui_init(void)
{
	ui_font_black = tf_new(&font_OpenSans_Regular_11X12, COLOR_BLACK, 0, TF_WORDWRAP);
	ui_font_white = tf_new(&font_OpenSans_Regular_11X12, COLOR_WHITE, 0, TF_WORDWRAP);
	ui_font_red = tf_new(&font_OpenSans_Regular_11X12, COLOR_RED, 0, TF_WORDWRAP);
}

void ui_free(void)
{
	tf_free(ui_font_black);
	tf_free(ui_font_white);
	tf_free(ui_font_red);
}

void ui_message_error(const char *msg)
{
	const int ypos = 112;
	const rect_t r = (rect_t){.x = 0, .y = ypos, .width = DISPLAY_WIDTH, .height = 16};
	fill_rectangle(fb, r, COLOR_RED);
	tf_draw_str(fb, ui_font_white, msg, (point_t){.x = 3, .y = ypos + 3});
	fprintf(stderr, "error: %s\n", msg);
	display_update_rect(r);

	event_t event;
	for (;;) {
		wait_event(&event);
		if (event.type == EVENT_TYPE_QUIT || (event.type == EVENT_TYPE_KEYPAD && event.keypad.pressed))
			break;
	}
}

void ui_draw_pathbar(const char *left, const char *right, bool fruncate)
{
	assert(left != NULL);

	fill_rectangle(fb, (rect_t){.x = 0, .y = 16, .width = DISPLAY_WIDTH, .height = 15}, 0xFFFF);

	// Draw left side
	const int max_left_len = 35;
	char left_buf[max_left_len + 1];
	const size_t left_len = strlen(left);
	// Eventually truncate left size
	if (left_len > max_left_len) {
		if (fruncate) {
			fruncate_str(left_buf, left, max_left_len);
		} else {
			truncate_str(left_buf, left, max_left_len);
		}
		left = left_buf;
	}
	tf_draw_str(fb, ui_font_black, left, (point_t){.x = 3, .y = 18});

	// Draw right side if present
	if (right != NULL) {
		const tf_metrics_t m = tf_get_str_metrics(ui_font_black, right);
		tf_draw_str(fb, ui_font_black, right, (point_t){.x = DISPLAY_WIDTH - m.width - 3, .y = 18});
	}

	display_update_rect((rect_t){.x = 0, .y = 16, .width = DISPLAY_WIDTH, .height = 15});
}
