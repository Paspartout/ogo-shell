#include <event.h>
#include <keypad.h>
#include <assert.h>

#include <SDL2/SDL.h>

// TODO: Support for multikeypresses
// Map the sdl keycode to odroid keypad enum/constant/flag
static uint16_t map_sdl_keysym(int keycode)
{
	switch (keycode) {
	case SDLK_UP:
	case SDLK_w:
		return KEYPAD_UP;
	case SDLK_RIGHT:
	case SDLK_d:
		return KEYPAD_RIGHT;
	case SDLK_DOWN:
	case SDLK_s:
		return KEYPAD_DOWN;
	case SDLK_LEFT:
	case SDLK_a:
		return KEYPAD_LEFT;
	case SDLK_COMMA:
		return KEYPAD_SELECT;
	case SDLK_PERIOD:
		return KEYPAD_START;
	case SDLK_RETURN:
	case SDLK_x:
		return KEYPAD_A;
	case SDLK_BACKSPACE:
	case SDLK_y:
		return KEYPAD_B;
	case SDLK_m:
	case SDLK_PLUS:
		return KEYPAD_MENU;
	case SDLK_v:
	case SDLK_HASH:
		return KEYPAD_VOLUME;
	default:
		return 0;
	}
}

static Uint32 OdroidEventType = ((Uint32)-1);

void event_init(void)
{
	OdroidEventType = SDL_RegisterEvents(1);
	assert(OdroidEventType != ((Uint32)-1));
}

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
			if (!e.key.repeat)
				event->keypad.pressed = map_sdl_keysym(e.key.keysym.sym);
			else
				event->keypad.pressed = 0;
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
			if (e.type == OdroidEventType) {
				assert(e.user.data1 != NULL);
				memcpy(event, e.user.data1, sizeof(event_t));
				free(e.user.data1);
				break;
			} else {
				continue;
			}
		}
		return 1;
	}
}

int push_event(event_t *event)
{
	SDL_Event e = {
	    0,
	};
	e.type = OdroidEventType;
	e.user.data1 = malloc(sizeof(event_t));
	memcpy(e.user.data1, event, sizeof(event_t));
	return SDL_PushEvent(&e);
}
