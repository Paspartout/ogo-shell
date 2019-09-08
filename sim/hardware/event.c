#include <event.h>
#include <keypad.h>

#include <SDL2/SDL.h>

// Map the sdl keycode to odroid keypad enum/constant/flag
static uint16_t map_sdl_keysym(int keycode)
{
	switch (keycode) {
	case SDLK_UP:
	case SDLK_w:
		return KEYPAD_UP;
		break;
	case SDLK_RIGHT:
	case SDLK_d:
		return KEYPAD_RIGHT;
		break;
	case SDLK_DOWN:
	case SDLK_s:
		return KEYPAD_DOWN;
		break;
	case SDLK_LEFT:
	case SDLK_a:
		return KEYPAD_LEFT;
		break;
	case SDLK_COMMA:
		return KEYPAD_SELECT;
		break;
	case SDLK_PERIOD:
		return KEYPAD_START;
		break;
	case SDLK_RETURN:
	case SDLK_x:
		return KEYPAD_A;
		break;
	case SDLK_BACKSPACE:
	case SDLK_y:
		return KEYPAD_B;
		break;
	case SDLK_m:
	case SDLK_PLUS:
		return KEYPAD_MENU;
		break;
	case SDLK_v:
	case SDLK_HASH:
		return KEYPAD_VOLUME;
		break;
	default:
		return 0;
		break;
	}
}

void event_init(void) {}

int wait_event(event_t *event)
{
	static SDL_Event e;

	for (;;) {
		if (!SDL_WaitEvent(&e)) {
			return -1;
		}

		switch (e.type) {
		case SDL_KEYDOWN:
			event->type = EVENT_TYPE_KEYPAD;
			event->keypad.released = 0;
			event->keypad.pressed = map_sdl_keysym(e.key.keysym.sym);
			break;
		case SDL_KEYUP:
			event->type = EVENT_TYPE_KEYPAD;
			event->keypad.pressed = 0;
			event->keypad.released = map_sdl_keysym(e.key.keysym.sym);
			break;
		case SDL_QUIT:
			event->type = EVENT_TYPE_QUIT;
			break;
		case SDL_WINDOWEVENT:
			event->type = EVENT_TYPE_UPDATE;
			break;
		default:
			continue;
			break;
		}
		return 1;
	}
}