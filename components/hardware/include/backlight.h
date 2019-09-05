#ifndef BACKLIGHT_H
#define BACKLIGHT_H

void backlight_init(void);
void backlight_percentage_set(int value);
int is_backlight_initialized(void);

#endif /* BACKLIGHT_H */