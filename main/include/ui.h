#pragma once

#include <tf.h>

#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_GRAY 0x4A69
#define COLOR_ORANGE 0xDB60
#define COLOR_RED 0xF800

extern tf_t *ui_font_black;
extern tf_t *ui_font_red;
extern tf_t *ui_font_white;

/// Load/confiugure ui
void ui_init(void);
/// Free ui resources
void ui_free(void);

/// Display an error message
void ui_message_error(const char *msg);
/// Draw the pathbar that is located under the status bar
void ui_draw_pathbar(const char *left, const char *right, bool fruncate);
