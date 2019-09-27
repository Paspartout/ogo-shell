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
#include <unistd.h>

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

#include <acodecs.h>

static void player_task(void *arg);

// Player state
static char *filename = NULL;
static bool playing = true;
static AudioOutput output = AudioOutputSpeaker;

static bool backlight = true;

// TODO: Unify using font module/compilation unit
static tf_t *font_white;

static void draw_player(void)
{
	fill_rectangle(fb, (rect_t){.x = 0, .y = 32, .width = DISPLAY_WIDTH, .height = DISPLAY_HEIGHT - 32}, COLOR_GRAY);

	const int line_height = 16;
	short y = 34;
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

	// TODO: Only update rect that updated
	display_update();
}

static AudioCodec choose_codec(FileType ftype)
{
	switch (ftype) {
	case FileTypeMOD:
		return AudioCodecMOD;
	case FileTypeMP3:
		return AudioCodecMP3;
	case FileTypeOGG:
		return AudioCodecOGG;
	case FileTypeFLAC:
		return AudioCodecFLAC;
	case FileTypeWAV:
		return AudioCodecWAV;
	default:
		return AudioCodecUnknown;
	}
}

typedef enum PlayerCmd {
	PlayerCmdNone,
	PlayerCmdTerminate,
	PlayerCmdPause,
} PlayerCmd;

typedef struct PlayerParam {
	char filepath[PATH_MAX];
	AudioCodec codec;
	AudioDecoder *decoder;
} PlayerParam;

#ifndef SIM

static QueueHandle_t player_cmd_queue;
static QueueHandle_t player_ack_queue;
static TaskHandle_t audio_player_task_handle;

static PlayerCmd player_poll_cmd(void)
{
	PlayerCmd polled_cmd = PlayerCmdNone;

	xQueueReceive(player_cmd_queue, &polled_cmd, 0);
	return polled_cmd;
}

static void player_send_cmd(PlayerCmd cmd) {
	xQueueSend(player_cmd_queue, &cmd, 0);
	int tmp;
	xQueueReceive(player_ack_queue, &tmp, portMAX_DELAY);
}

static void player_cmd_ack(void) {
	int tmp = 42;
	xQueueSend(player_ack_queue, &tmp, 0);
}

static void audio_player_start(PlayerParam *param)
{
	const int stacksize = 9 * 8192; // dr_mp3 uses a lot of stack memory
	if (xTaskCreate(player_task, "player_task", stacksize, param, 5, &audio_player_task_handle) != pdPASS) {
		printf("Error creating task\n");
		return;
	}
	player_cmd_queue = xQueueCreate(1, sizeof(PlayerCmd));
	player_ack_queue = xQueueCreate(1, sizeof(int));
}

static void audio_player_terminate(void)
{
	printf("Trying to terminate player..\n");
	PlayerCmd term = PlayerCmdTerminate;
	xQueueSend(player_cmd_queue, &term, portMAX_DELAY);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	printf("Terminated?\n");
}

static void player_teardown_task()
{
	vTaskDelete(NULL);
	// Nothing todo here
}

#else

// Used to simulate task
static SDL_Thread *thread = NULL;

// Used for cmd communication
static SDL_mutex *cmd_mut = NULL;
static PlayerCmd sent_cmd = PlayerCmdNone;

static PlayerCmd player_poll_cmd(void)
{
	SDL_LockMutex(cmd_mut);
	PlayerCmd polled_cmd = sent_cmd;
	SDL_UnlockMutex(cmd_mut);

	return polled_cmd;
}

static void player_cmd_ack(void) {
	SDL_LockMutex(cmd_mut);
	sent_cmd = PlayerCmdNone;
	SDL_UnlockMutex(cmd_mut);
}

static void player_send_cmd(PlayerCmd cmd)
{
	SDL_LockMutex(cmd_mut);
	if (sent_cmd != PlayerCmdNone) {
		SDL_UnlockMutex(cmd_mut);
		return;
	}
	sent_cmd = cmd;
	SDL_UnlockMutex(cmd_mut);


	PlayerCmd received_cmd;
	do {
	    received_cmd = sent_cmd;
	    usleep(100);
	} while(received_cmd != PlayerCmdNone);
}

static void player_teardown_task()
{
	// Nothing todo here
}

static int sdl_player_task(void *param)
{
	player_task(param);
	return 0;
}

static void audio_player_start(PlayerParam *param)
{
	thread = SDL_CreateThread(sdl_player_task, "player_task", param);
	assert(thread != NULL);
	cmd_mut = SDL_CreateMutex();
	assert(cmd_mut != NULL);
	playing = true;

	player_send_cmd(PlayerCmdNone);
}

static void audio_player_terminate(void) { player_send_cmd(PlayerCmdTerminate); }

#endif

/** This task/thread plays one particular audio file. */
static void player_task(void *arg)
{
	struct PlayerParam *param = arg;
	const char *fpath = param->filepath;
	const AudioDecoder *decoder = param->decoder;
	printf("Playing file: %s, codec: %d\n", fpath, param->codec);

	void *acodec_handle = NULL;
	if (decoder->open(&acodec_handle, fpath) != 0) {
		// TODO: Error handling
		fprintf(stderr, "Error opening audio file\n");
	}
	AudioInfo info;
	decoder->get_info(acodec_handle, &info);

	int16_t *audio_buf = calloc(1, info.buf_size * sizeof(uint16_t));
	int n_frames = 0;

	audio_init((int)info.sample_rate, output);

	playing = true;
	do {
		// React on user control
		PlayerCmd received_cmd = player_poll_cmd();
		if (received_cmd != PlayerCmdNone) {
			printf("Received cmd: %d\n", received_cmd);
			if (received_cmd == PlayerCmdPause) {
				playing = !playing;
				if (playing) {
					audio_init((int)info.sample_rate, output);
				} else {
					audio_shutdown();
				}
				player_cmd_ack();
			} else if (received_cmd == PlayerCmdTerminate) {
				player_cmd_ack();
				break;
			}
		}
		// Play if not paused
		if (playing) {
			n_frames = decoder->decode(acodec_handle, audio_buf, (int)info.channels, info.buf_size);
			audio_submit(audio_buf, n_frames);
		} else {
			// TODO: Delay?
		}
	} while (n_frames > 0);
	printf("Terminating player task...\n");
	decoder->close(acodec_handle);
	free(audio_buf);

	if (playing)
		audio_shutdown();

	player_teardown_task();
}

static void handle_keypress(uint16_t keys, bool *quit)
{
	float audio_volume_new = 0;
	switch (keys) {
	case KEYPAD_A:
		// TODO: Pause/Play
		player_send_cmd(PlayerCmdPause);
		draw_player();
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
	default:
		// TODO: Bubble up to parent
		break;
	} // switch(event.keypad.pressed)
}

int audio_player(Entry *entries, int index, const char *cwd)
{
	// Open file and start playing in seperate task
	PlayerParam param;
	filename = entries[index].name;
	snprintf(param.filepath, PATH_MAX, "%s/%s", cwd, filename);
	param.codec = choose_codec(fops_determine_filetype(param.filepath));
	param.decoder = acodec_get_decoder(param.codec);
	assert(param.decoder != NULL);
	assert(param.codec != AudioCodecUnknown);
	// TODO: Show error if can't determine coodec/filetype/decoder

	// Start playing the slected file
	printf("Trying to open audio file: %s\n", param.filepath);
	audio_player_start(&param);

	// Draw player interface before opening file
	font_white = tf_new(&font_OpenSans_Regular_11X12, 0xFFFF, 0, TF_WORDWRAP);
	draw_player();

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
