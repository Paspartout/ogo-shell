#include <ui.h>

#include <display.h>
#include <gbuf.h>
#include <tf.h>
#include <graphics.h>
#include <event.h>
#include <OpenSans_Regular_11X12.h>

#include <stdio.h>

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

// TODO: printf like behaviour would be nice for detailed error messages
void ui_message_error(const char *msg)
{
	const int ypos = 110;
	const rect_t r = (rect_t){.x = 0, .y = ypos, .width = DISPLAY_WIDTH, .height = 16};
	fill_rectangle(fb, r, COLOR_RED);
	tf_draw_str(fb, ui_font_white, msg, (point_t){.x = 3, .y = ypos + 3});
	fprintf(stderr, "error: %s\n", msg);
	display_update_rect(r);

	event_t event;
	for (;;) {
		wait_event(&event);
		if (event.type == EVENT_TYPE_KEYPAD && event.keypad.pressed)
			break;
	}
}
