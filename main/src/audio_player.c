#include <audio_player.h>

#include <OpenSans_Regular_11X12.h>
#include <event.h>
#include <file_ops.h>
#include <gbuf.h>
#include <graphics.h>
#include <keypad.h>
#include <tf.h>

#include <display.h>
#include <audio.h>
#include <backlight.h>

#include <limits.h>

#ifndef SIM
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#else
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL.h>
#include <assert.h>
#endif

#define COLOR_GRAY 0x4A69

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.h>
#include <dr_mp3.h>

// Player state
static char *filename = NULL;
static char filepath[PATH_MAX];
static bool playing = true;
static AudioOutput output = AudioOutputDAC;

static bool backlight = true;

// TODO: Unify using font module/compilation unit
static tf_t *font_white;

static void draw_player(void)
{
	fill_rectangle(fb, (rect_t){.x = 0, .y = 32, .width = DISPLAY_WIDTH, .height = DISPLAY_HEIGHT - 32}, COLOR_GRAY);

	const int line_height = 16;
	int y = 34;
	tf_draw_str(fb, font_white, "Audio Player", (point_t){.x = 3, .y = y});
	y += line_height;
	//  Song name
	char str_buf[300];
	snprintf(str_buf, 300, "File: %s", filename);
	tf_draw_str(fb, font_white, str_buf, (point_t){.x = 3, .y = y});
	y += line_height * 2;

	// Show Playing or paused
	tf_draw_str(fb, font_white, playing ? "State: Playing" : "State: Paused", (point_t){.x = 3, .y = y});
	y += line_height;

	// TODO: Song position/duration?
	// TODO: Controls, etc

	display_update();
}

typedef enum PlayerCmd {
	PlayerCmdNone,
	PlayerCmdTerminate,
	PlayerCmdPause,
} PlayerCmd;

static PlayerCmd sent_cmd = PlayerCmdNone;

#ifndef SIM

static QueueHandle_t audio_cmd_queue;
static TaskHandle_t audio_player_task_handle;

void audio_player_task(void *arg)
{
	const char *fpath = (char *)arg;
	FileType ftype = fops_determine_filetype(fpath);
	printf("Opening file: %s, ftype: %d\n", fpath, ftype);
	short audio_buf[4096];

	if (ftype == FileTypeOGG) {
		int error;
		stb_vorbis *vorbis;
		vorbis = stb_vorbis_open_filename(fpath, &error, NULL);
		stb_vorbis_info info = stb_vorbis_get_info(vorbis);
		audio_init(info.sample_rate, output);

		PlayerCmd cmd;
		int n_frames = 0;
		playing = true;
		do {
			// React on user control
			if (xQueueReceive(audio_cmd_queue, &cmd, 0) == pdTRUE) {
				if (cmd == PlayerCmdPause) {
					playing = !playing;
					if (playing) {
						audio_init(info.sample_rate, output);
					} else {
						audio_shutdown();
					}
				} else if (cmd == PlayerCmdTerminate) {
					break;
				}
			}
			// Play if not paused
			if (playing) {
				n_frames = stb_vorbis_get_frame_short_interleaved(vorbis, info.channels, audio_buf, 4096);
				audio_submit(audio_buf, n_frames);
			} else {
				vTaskDelay(10 / portTICK_PERIOD_MS);
			}
		} while (n_frames > 0);
		stb_vorbis_close(vorbis);
	} else if (ftype == FileTypeMP3) {
		// TODO: Cleanup and merge with OGG
		// Too tired for proper coding atm just wanna have mp3 support
		int sample_rate = 0;
		drmp3 mp3;
		if (!drmp3_init_file(&mp3, fpath, NULL)) {
			printf("Error opening mp3 file :/\n");
		}
		sample_rate = mp3.sampleRate;
		audio_init(sample_rate, output);

		PlayerCmd cmd;
		int n_frames = 0;
		playing = true;
		do {
			// React on user control
			if (xQueueReceive(audio_cmd_queue, &cmd, 0) == pdTRUE) {
				if (cmd == PlayerCmdPause) {
					playing = !playing;
					if (playing) {
						audio_init(sample_rate, output);
					} else {
						audio_shutdown();
					}
				} else if (cmd == PlayerCmdTerminate) {
					break;
				}
			}
			// Play if not paused
			if (playing) {
				n_frames = drmp3_read_pcm_frames_s16(&mp3, 2048, audio_buf);
				audio_submit(audio_buf, n_frames);
			} else {
				vTaskDelay(10 / portTICK_PERIOD_MS);
			}
		} while (n_frames > 0);
		drmp3_uninit(&mp3);
	}

	// TODO: Teardown
	audio_shutdown();
	for (;;) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void audio_player_start(void)
{
	if (xTaskCreate(audio_player_task, "player_task", 9 * 8192, filepath, 5, &audio_player_task_handle) != pdPASS) {
		printf("Error creating task\n");
		return;
	}
	audio_cmd_queue = xQueueCreate(3, sizeof(PlayerCmd));
}

void audio_player_pause(void)
{
	PlayerCmd term = PlayerCmdPause;
	xQueueSend(audio_cmd_queue, &term, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS); // TODO: Proper sync
}

void audio_player_terminate(void)
{
	printf("Trying to terminate player..\n");
	PlayerCmd term = PlayerCmdTerminate;
	xQueueSend(audio_cmd_queue, &term, portMAX_DELAY);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	vTaskDelete(audio_player_task_handle);
	printf("Terminated?\n");
}

#else

static SDL_Thread *thread = NULL;
static SDL_mutex *cmd_mut = NULL;

int audio_player_task(void *arg)
{
	const char *fpath = (char *)arg;
	FileType ftype = fops_determine_filetype(fpath);
	printf("Opening file: %s, ftype: %d\n", fpath, ftype);
	short audio_buf[4096];

	if (ftype == FileTypeOGG) {
		int error;
		stb_vorbis *vorbis;
		vorbis = stb_vorbis_open_filename(fpath, &error, NULL);
		stb_vorbis_info info = stb_vorbis_get_info(vorbis);
		audio_init(info.sample_rate, output);

		int n_frames = 0;
		playing = true;
		do {
			// React on user control
			SDL_LockMutex(cmd_mut);
			if (sent_cmd != PlayerCmdNone) {
				printf("Received cmd: %d\n", sent_cmd);
				if (sent_cmd == PlayerCmdPause) {
					playing = !playing;
					if (playing) {
						audio_init(info.sample_rate, output);
					} else {
						audio_shutdown();
					}
				} else if (sent_cmd == PlayerCmdTerminate) {
					break;
				}
				sent_cmd = PlayerCmdNone;
			}
			SDL_UnlockMutex(cmd_mut);
			// Play if not paused
			if (playing) {
				n_frames = stb_vorbis_get_frame_short_interleaved(vorbis, info.channels, audio_buf, 4096);
				audio_submit(audio_buf, n_frames);
			} else {
				SDL_Delay(10);
			}
		} while (n_frames > 0);
		stb_vorbis_close(vorbis);
		if (playing)
			audio_shutdown();
	}
	return 0;
}

void audio_player_start(void)
{
	thread = SDL_CreateThread(audio_player_task, "player_task", (void *)filepath);
	assert(thread != NULL);
	cmd_mut = SDL_CreateMutex();
	assert(cmd_mut != NULL);
	SDL_LockMutex(cmd_mut);
	sent_cmd = PlayerCmdNone;
	printf("Set to %d\n", sent_cmd);
	SDL_UnlockMutex(cmd_mut);
}

void audio_player_pause(void)
{
	SDL_LockMutex(cmd_mut);
	sent_cmd = PlayerCmdPause;
	SDL_UnlockMutex(cmd_mut);
}

void audio_player_terminate(void)
{
	SDL_LockMutex(cmd_mut);
	sent_cmd = PlayerCmdTerminate;
	SDL_UnlockMutex(cmd_mut);
}

#endif

static void handle_keypress(uint16_t keys, bool *quit)
{
	float audio_volume_new = 0;
	switch (keys) {
	case KEYPAD_A:
		// TODO: Pause/Play
		audio_player_pause();
		break;
	case KEYPAD_B:
		*quit = true;
		break;
	case KEYPAD_UP:
		audio_volume_new = audio_volume + 0.1f;
		if (audio_volume_new > 1.0f) {
			audio_volume_new = 1.0f;
		}
		audio_volume = audio_volume_new;
		// TODO: Vol up?
		break;
	case KEYPAD_DOWN:
		audio_volume_new = audio_volume - 0.1f;
		if (audio_volume_new < 0.0f) {
			audio_volume_new = 0.0f;
		}
		audio_volume = audio_volume_new;
		// TODO: Vol down?
		break;
	case KEYPAD_RIGHT:
		// TODO: Next song
		break;
	case KEYPAD_LEFT:
		// TODO: Prev song
		break;
	case KEYPAD_VOLUME:
		// TODO: Vol toggle?
		break;
	case KEYPAD_START:
		// TODO: Stop/Restart Song
		if (output == AudioOutputSpeaker)
			output = AudioOutputDAC;
		else
			output = AudioOutputSpeaker;
		break;
	case KEYPAD_SELECT:
		backlight = !backlight;
		backlight_percentage_set(backlight ? 50 : 0);
		break;
	} // switch(event.keypad.pressed)
}

int audio_player(Entry *entries, int index, const char *cwd)
{
	// Open file and start playing in seperate task
	filename = entries[index].name;
	snprintf(filepath, PATH_MAX, "%s/%s", cwd, filename);
	printf("Trying to open audio file: %s\n", filepath);

	font_white = tf_new(&font_OpenSans_Regular_11X12, 0xFFFF, 0, TF_WORDWRAP);
	draw_player();

	audio_player_start();

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
		}
	}

	audio_player_terminate();

	tf_free(font_white);
	// TODO: Deinit audio decoder and driver
	return 0;
}
