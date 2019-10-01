#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <audio.h>

#include <SDL2/SDL.h>

static bool initialized = false;
static SDL_AudioDeviceID audio_device = 0;
static int audio_volume = 50;
static float audio_volume_f = 50;

// TODO: Implement simulation using SDL

/** Initialize audio driver/i2s with given sample rate and output mode. */
int audio_init(int sample_rate, const AudioOutput output)
{
	assert(!initialized);
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "error initializing SDL Audio: %s\n", SDL_GetError());
		return -1;
	}

	SDL_AudioSpec want = {0}, have = {0};
	want.freq = sample_rate;
	want.format = AUDIO_S16;
	want.channels = 2;
	want.callback = NULL;

	if ((audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE)) == 0) {
		fprintf(stderr, "Error opening audio device: %s\n", SDL_GetError());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return -1;
	}

	if (have.format != want.format) {
		fprintf(stderr, "Got wrong fromat: %d\n", have.format);
		return -1;
	}

	printf("Initialized audio: sr=%d output=%d\n", sample_rate, output);
	SDL_PauseAudioDevice(audio_device, 0);
	initialized = true;
	return 0;
}

/** Shutdown audio driver. */
int audio_shutdown(void)
{
	assert(initialized);
	printf("Shutting audio down\n");
	SDL_CloseAudioDevice(audio_device);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	audio_device = 0;
	initialized = false;
	return 0;
}

/** Submit audio. */
void audio_submit(short *buf, int len)
{
	assert(initialized);
	assert(buf != NULL);
	assert(len >= 0);

	SDL_QueueAudio(audio_device, buf, 2 * (uint32_t)len * sizeof(*buf));
	uint32_t left = 0;
	do {
		left = SDL_GetQueuedAudioSize(audio_device);
		SDL_Delay(1);
	} while (left > 4096);
}

int audio_volume_set(int volume_percent)
{
	if (volume_percent > 100)
		volume_percent = 100;
	else if (volume_percent < 0)
		volume_percent = 0;
	audio_volume = volume_percent;
	audio_volume_f = (float)volume_percent / 100.0f;
	return audio_volume;
}

int audio_volume_get(void) { return audio_volume; }
