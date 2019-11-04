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
#include <ui.h>
#include <settings.h>
#include <guide_img.c>
#include <system.h>

#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

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

#include <acodecs.h>

// TODO: Move these
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
	case FileTypeGME:
		return AudioCodecGME;
	default:
		return AudioCodecUnknown;
	}
}

typedef struct {
	char *filename;
	char *filepath;
	AudioCodec codec;
	// TODO: Add more information in later versions
} Song;

// Cmds sent to player task for control
typedef enum PlayerCmd {
	PlayerCmdNone,
	PlayerCmdTerminate,

	PlayerCmdPause,
	PlayerCmdNext,
	PlayerCmdPrev,
	PlayerCmdReinitAudio,
	PlayerCmdToggleLoopMode,
} PlayerCmd;

typedef enum PlayingMode {
	PlayingModeNormal = 0,
	PlayingModeRepeatSong,
	PlayingModeRepeatPlaylist,
	PlayingModeMax,
} PlayingMode;
static const char *playing_mode_str[PlayingModeMax] = {"Normal", "Repeat Song", "Repeat Playlist/Folder"};

// Owned by player task, can only be modified/written to by player task
typedef struct PlayerState {
	// Can be modified though Playing/Pause Cmd
	bool playing;

	// Can be modified? by SetPlayList Cmd?
	// NOTE: Maybe make playlist a more dynamic array for later versions
	Song *playlist;
	size_t playlist_length;

	// Marks current song index in playlist
	// Can be modified through PlayNext/PlayPrev
	int playlist_index;

	// TODO: Settings, should probably saved
	PlayingMode playing_mode;
} PlayerState;
static PlayerState player_state = {
    0,
};
static bool keys_locked = false;
static bool backlight_on = true;

// These need to be implemented for SDL/FreeRTOS seperately
static PlayerCmd player_poll_cmd(void);
static void player_send_cmd(PlayerCmd cmd);
static void player_cmd_ack(void);
static void player_start(void);
static void player_terminate(void);
static void player_teardown_task(void);

static void player_task(void *arg);

static bool player_task_running = false;

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

static void player_send_cmd(PlayerCmd cmd)
{
	xQueueSend(player_cmd_queue, &cmd, 0);
	int tmp;
	xQueueReceive(player_ack_queue, &tmp, 10 / portTICK_PERIOD_MS);
}

static void player_cmd_ack(void)
{
	int tmp = 42;
	xQueueSend(player_ack_queue, &tmp, 0);
}

static void player_start(void)
{
	const int stacksize = 9 * 8192; // dr_mp3 uses a lot of stack memory
	if (xTaskCreate(player_task, "player_task", stacksize, NULL, 5, &audio_player_task_handle) != pdPASS) {
		printf("Error creating task\n");
		return;
	}
	player_cmd_queue = xQueueCreate(3, sizeof(PlayerCmd));
	player_ack_queue = xQueueCreate(3, sizeof(int));
}

static void player_terminate(void)
{
	if (!player_task_running) {
		return;
	}
	printf("Trying to terminate player..\n");
	PlayerCmd term = PlayerCmdTerminate;
	xQueueSend(player_cmd_queue, &term, portMAX_DELAY);
	while (player_task_running) {
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	printf("Terminated?\n");
}

static void player_teardown_task(void) { vTaskDelete(NULL); }

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

static void player_cmd_ack(void)
{
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
	for (int i = 0; i < 100; i++) {
		received_cmd = sent_cmd;
		if (received_cmd == PlayerCmdNone) {
			break;
		}
		usleep(1000);
	}
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

static void player_start(void)
{
	assert(!player_task_running);
	thread = SDL_CreateThread(sdl_player_task, "player_task", NULL);
	assert(thread != NULL);
	cmd_mut = SDL_CreateMutex();
	assert(cmd_mut != NULL);
	player_send_cmd(PlayerCmdNone);
}

static void player_terminate(void)
{
	if (!player_task_running) {
		return;
	}
	player_send_cmd(PlayerCmdTerminate);
	while (player_task_running) {
		usleep(10 * 1000);
	}
	SDL_DestroyMutex(cmd_mut);
}

#endif

typedef enum PlayerResult {
	PlayerResultNone = 0,
	PlayerResultError,

	PlayerResultDone,     ///< Playing the song completed, same effect as NextSong
	PlayerResultNextSong, ///< User requested skipping of next song
	PlayerResultPrevSong, ///< User requested to play previous song
	PlayerResultStop,     ///< User exited player/requested player termination
} PlayerResult;

static void push_audio_event(const AudioPlayerEvent audio_event)
{
	event_t ev = {.audio_player.head.type = EVENT_TYPE_AUDIO_PLAYER, .audio_player.event = audio_event};
	push_event(&ev);
}

/// Handle a playercmd, return 1 if we should stop?
static PlayerResult handle_cmd(PlayerState *const state, const AudioInfo info, const PlayerCmd received_cmd)
{
	if (received_cmd == PlayerCmdNone) {
		return PlayerResultDone;
	}
	fprintf(stderr, "Received cmd: %d\n", received_cmd);
	PlayerResult res = PlayerResultDone;
	switch (received_cmd) {
	case PlayerCmdPause:
		state->playing = !state->playing;
		if (state->playing) {
			audio_init((int)info.sample_rate, audio_output_get());
		} else {
			audio_shutdown();
		}
		push_audio_event(AudioPlayerEventStateChanged);
		break;
	case PlayerCmdReinitAudio:
		if (state->playing)
			audio_shutdown();
		audio_init((int)info.sample_rate, audio_output_get());
		push_audio_event(AudioPlayerEventStateChanged);
		break;
	case PlayerCmdToggleLoopMode:
		state->playing_mode = (state->playing_mode + 1) % PlayingModeMax;
		push_audio_event(AudioPlayerEventStateChanged);
		break;
	case PlayerCmdTerminate:
		res = PlayerResultStop;
		break;
	case PlayerCmdNext:
		res = PlayerResultNextSong;
		break;
	case PlayerCmdPrev:
		res = PlayerResultPrevSong;
		break;
	case PlayerCmdNone:
		break;
	}
	player_cmd_ack();
	return res;
}

static PlayerResult play_song(const Song *const song)
{
	PlayerState *state = &player_state;
	AudioInfo info;
	void *acodec = NULL;

	printf("Playing file: %s, codec: %d\n", song->filepath, song->codec);
	AudioDecoder *decoder = acodec_get_decoder(song->codec);
	if (decoder == NULL) {
		fprintf(stderr, "error determining deocer for song %s\n", song->filepath);
		return PlayerResultError;
	}

	if (decoder->open(&acodec, song->filepath) != 0) {
		fprintf(stderr, "error opening song %s\n", song->filepath);
		return PlayerResultError;
	}
	if (decoder->get_info(acodec, &info) != 0) {
		decoder->close(acodec);
		fprintf(stderr, "error retreiving song info %s\n", song->filepath);
		return PlayerResultError;
	}

	int16_t *audio_buf = calloc(1, info.buf_size * sizeof(uint16_t));
	if (audio_buf == NULL) {
		decoder->close(acodec);
		fprintf(stderr, "error allocating audio buffer\n");
		return PlayerResultError;
	}

	audio_init((int)info.sample_rate, audio_output_get());

	int n_frames = 0;
	state->playing = true;
	fprintf(stderr, "Pushing audio event...\n");
	push_audio_event(AudioPlayerEventStateChanged);

	PlayerResult result = PlayerResultDone;
	fprintf(stderr, "starting to play audio...\n");
	do {
		// React on user control
		if ((result = handle_cmd(state, info, player_poll_cmd())) != PlayerResultDone) {
			break;
		}

		// Play if not paused
		// TODO: Fix mono playback
		if (state->playing) {
			n_frames = decoder->decode(acodec, audio_buf, (int)info.channels, info.buf_size);
			audio_submit(audio_buf, n_frames);
		} else {
			usleep(10 * 1000);
		}
	} while (n_frames > 0);

	// TODO: Notify user that we are done
	decoder->close(acodec);
	free(audio_buf);

	if (state->playing) {
		audio_shutdown();
	}
	return result;
}

/** This task/thread plays a playlist of files automatically. */
static void player_task(void *arg)
{
	player_task_running = true;
	player_state.playing = false;
	struct PlayerState *state = &player_state;
	printf("Playing playlist of length: %zu\n", state->playlist_length);

	for (;;) {
		int song_index = state->playlist_index;
		Song *const song = &state->playlist[song_index];
		PlayerResult res = play_song(song);

		if (res == PlayerResultDone || res == PlayerResultNextSong) {
			if (state->playing_mode == PlayingModeNormal && song_index == (int)(state->playlist_length - 1)) {
				// Reached last song in playlist. Quit player.
				push_audio_event(AudioPlayerEventDone);
				break;
			}

			if (state->playing_mode != PlayingModeRepeatSong || res == PlayerResultNextSong) {
				song_index = (song_index + 1) % (int)state->playlist_length;
			}

			state->playlist_index = song_index;
			push_audio_event(AudioPlayerEventStateChanged);
		} else if (res == PlayerResultPrevSong) {
			if (--song_index < 0) {
				song_index = (int)state->playlist_length - 1;
			}
			state->playlist_index = song_index;
			push_audio_event(AudioPlayerEventStateChanged);
		} else if (res == PlayerResultStop) {
			push_audio_event(AudioPlayerEventDone);
			break;
		} else if (res == PlayerResultError) {
			push_audio_event(AudioPlayerEventError);
			break;
		}
	}

	player_task_running = false;
	player_teardown_task();
}

static void draw_player(const PlayerState *const state)
{
	rect_t window_rect = (rect_t){.x = 0, .y = 32, .width = DISPLAY_WIDTH, .height = DISPLAY_HEIGHT - 32};
	fill_rectangle(fb, window_rect, COLOR_GRAY);

	const int line_height = 16;
	short y = 34;

	char str_buf[300];
	Song *song = &state->playlist[state->playlist_index];

	snprintf(str_buf, 300, "%d/%d", state->playlist_index + 1, (int)state->playlist_length);
	ui_draw_pathbar("Audio Player", str_buf, false);

	// Song name
	snprintf(str_buf, 300, "Song: %s", song->filename);
	tf_draw_str(fb, ui_font_white, str_buf, (point_t){.x = 3, .y = y});
	y += line_height;

	// Song playmode
	snprintf(str_buf, 300, "Playing Mode: %s", playing_mode_str[state->playing_mode]);
	tf_draw_str(fb, ui_font_white, str_buf, (point_t){.x = 3, .y = y});
	y += line_height;

	// Show volume
	snprintf(str_buf, 300, "Volume: %d%%", audio_volume_get());
	tf_draw_str(fb, ui_font_white, str_buf, (point_t){.x = 3, .y = y});
	y += line_height + 3;

	// Show Playing or paused/DAC on image
	tf_draw_str(fb, ui_font_white, state->playing ? "Pause" : "Continue", (point_t){.x = 222, .y = y + 10});
	tf_draw_str(fb, ui_font_white, "Go back", (point_t){.x = 222, .y = y + 10 + 37});
	tf_draw_str(fb, ui_font_white, audio_output_get() == AudioOutputSpeaker ? "Switch DAC" : "Switch Speaker",
		    (point_t){.x = 222, .y = y + 10 + 37 * 2});

	// Display help image
	gbuf_t img = {.width = (uint16_t)guide_img.width,
		      .height = (uint16_t)guide_img.height,
		      .bytes_per_pixel = 2,
		      .data = (uint8_t *)&guide_img.pixel_data,
		      .big_endian = false};
	blit(fb, (rect_t){.x = 10, .y = y, .width = img.width, .height = img.height - 1}, &img,
	     (rect_t){.x = 0, .y = 1, .width = img.width, .height = img.height - 1});
	y += img.height + 8;

	// Explain start and stop button behaviour
	tf_draw_str(fb, ui_font_white, "SELECT: Toggle screen, hold(>1s) & release for Keylock", (point_t){.x = 3, .y = y});
	y += line_height + 3;
	tf_draw_str(fb, ui_font_white, "START: Cycle through Playing Mode", (point_t){.x = 3, .y = y});

	// TODO: Song position/duration?

	display_update_rect(window_rect);
}

/// Calculate the difference between two timespecs in milliseconds
static uint64_t ts_diff_ms(const struct timespec ts1, const struct timespec ts2)
{
	const uint64_t ts1_ms = ts1.tv_sec * (uint64_t)1000 + ts1.tv_nsec / (uint64_t)1000000;
	const uint64_t ts2_ms = ts2.tv_sec * (uint64_t)1000 + ts2.tv_nsec / (uint64_t)1000000;
	return ts2_ms - ts1_ms;
}

static void handle_keypress(event_keypad_t keys, bool *quit)
{
	static struct timespec menu_pressed_start, tmp;
	static bool select_pressed = false;
	if (keys.released) {
		if ((keys.released & KEYPAD_SELECT) && select_pressed) {
			clock_gettime(CLOCK_MONOTONIC, &tmp);
			const uint64_t diff = ts_diff_ms(menu_pressed_start, tmp);

			if (keys_locked) {
				if (diff > 1000) {
					system_led_set(true);
					usleep(100 * 1000);
					system_led_set(false);
					keys_locked = !keys_locked;
					backlight_on = !keys_locked;
				}
			} else {
				if (backlight_on && diff > 1000) {
					system_led_set(true);
					usleep(100 * 1000);
					system_led_set(false);
					keys_locked = !keys_locked;
					backlight_on = !keys_locked;
				} else {
					backlight_on = !backlight_on;
				}
			}

			select_pressed = false;
			backlight_percentage_set(backlight_on ? 50 : 0);
		}
	}

	if (keys_locked) {
		if (keys.pressed & KEYPAD_SELECT) {
			select_pressed = true;
			clock_gettime(CLOCK_MONOTONIC, &menu_pressed_start);
		}
		return;
	}

	switch (keys.pressed) {
	case KEYPAD_A:
		player_send_cmd(PlayerCmdPause);
		break;
	case KEYPAD_B:
		*quit = true;
		break;
	case KEYPAD_UP:
		// Volume up
		audio_volume_set(audio_volume_get() + 5);
		draw_player(&player_state);
		settings_save(SettingAudioVolume, audio_volume_get());
		break;
	case KEYPAD_DOWN:
		// Volume down
		audio_volume_set(audio_volume_get() - 5);
		draw_player(&player_state);
		settings_save(SettingAudioVolume, audio_volume_get());
		break;
	case KEYPAD_RIGHT:
		// Try to play next song in list
		player_send_cmd(PlayerCmdNext);
		break;
	case KEYPAD_LEFT:
		player_send_cmd(PlayerCmdPrev);
		break;
	case KEYPAD_VOLUME:
		// Toggle DAC/Speaker output mode
		audio_output_set(audio_output_get() == AudioOutputDAC ? AudioOutputSpeaker : AudioOutputDAC);
		player_send_cmd(PlayerCmdReinitAudio);
		settings_save(SettingAudioOutput, (int32_t)audio_output_get());
		break;
	case KEYPAD_START:
		// Toggle playing mode
		player_send_cmd(PlayerCmdToggleLoopMode);
		settings_save(SettingPlayingMode, (int32_t)player_state.playing_mode);
		break;
	case KEYPAD_SELECT:
		select_pressed = true;
		clock_gettime(CLOCK_MONOTONIC, &menu_pressed_start);
		break;
	case KEYPAD_MENU:
		// TODO: Show future system menu
		break;
	} // switch(event.keypad.pressed)

	if (!backlight_on) {
		system_led_set(true);
		usleep(100 * 1000);
		system_led_set(false);
	}
}

#define MAX_SONGS 1024

/// Make playlist out of entries in params and write them to state.
static int make_playlist(PlayerState *state, const AudioPlayerParam params)
{
	char pathbuf[PATH_MAX];

	if (params.play_all) {
		size_t start_song = 0;
		size_t n_songs = 0;
		size_t song_indices[MAX_SONGS];
		for (size_t i = 0, j = 0; i < (size_t)params.n_entries; i++) {
			Entry *entry = &params.entries[i];
			AudioCodec codec = choose_codec(fops_determine_filetype(entry));
			if (codec != AudioCodecUnknown) {
				if ((size_t)params.index == i) {
					start_song = j;
				}
				song_indices[j++] = i;
				n_songs++;
			}
		}

		state->playlist = malloc(n_songs * sizeof(Song));
		state->playlist_length = n_songs;

		for (size_t i = 0; i < n_songs; i++) {
			Entry *entry = &params.entries[song_indices[i]];

			AudioCodec codec = choose_codec(fops_determine_filetype(entry));
			state->playlist[i].codec = codec;

			int printed = snprintf(pathbuf, PATH_MAX, "%s/%s", params.cwd, entry->name);
			state->playlist[i].filename = malloc(strlen(entry->name) + 1);
			state->playlist[i].filepath = malloc((size_t)(printed + 1));
			strncpy(state->playlist[i].filename, entry->name, (size_t)(strlen(entry->name) + 1));
			strncpy(state->playlist[i].filepath, pathbuf, (size_t)(printed + 1));
		}
		state->playlist_index = (int)start_song;
	} else {
		Entry *entry = &params.entries[params.index];
		AudioCodec codec = choose_codec(fops_determine_filetype(entry));
		if (codec == AudioCodecUnknown) {
			return -1;
		}

		state->playlist = malloc(1 * sizeof(Song));
		state->playlist[0].codec = codec;
		state->playlist_length = 1;
		int printed = snprintf(pathbuf, PATH_MAX, "%s/%s", params.cwd, entry->name);
		state->playlist[0].filepath = malloc((size_t)(printed + 1));
		strncpy(state->playlist[0].filepath, pathbuf, (size_t)(printed + 1));
	}

	return 0;
}

/// Free previously allocated/created playlist.
static void free_playlist(PlayerState *state)
{
	const size_t len = state->playlist_length;
	if (len < 1)
		return;
	for (size_t i = 0; i < len; i++) {
		free(state->playlist[i].filepath);
		free(state->playlist[i].filename);
	}
	free(state->playlist);
}

static void load_settings(PlayerState *state)
{
	int32_t mode;
	if (settings_load(SettingPlayingMode, &mode) == 0) {
		if (mode >= 0 && mode <= PlayingModeMax)
			state->playing_mode = (PlayingMode)mode;
	}
}

int audio_player(AudioPlayerParam params)
{
	memset(&player_state, 0, sizeof(PlayerState));
	load_settings(&player_state);

	if (make_playlist(&player_state, params) != 0) {
		ui_message_error("Could not determine audio codec");
		return -1;
	}

	// Start playing the slected file
	player_start();

	bool quit = false;
	event_t event;
	while (!quit) {
		// Handle inputs
		if (wait_event(&event) < 0) {
			continue;
		}
		switch (event.type) {
		case EVENT_TYPE_KEYPAD:
			handle_keypress(event.keypad, &quit);
			break;
		case EVENT_TYPE_AUDIO_PLAYER:
			if (event.audio_player.event == AudioPlayerEventDone) {
				quit = true;
			} else if (event.audio_player.event == AudioPlayerEventError) {
				ui_message_error("Error playing audio file");
				quit = true;
			} else {
				draw_player(&player_state);
			}
			break;
		case EVENT_TYPE_QUIT:
			quit = true;
			break;
		case EVENT_TYPE_UPDATE:
			display_update();
			break;
		}
	}

	player_terminate();
	free_playlist(&player_state);
	keys_locked = false;
	backlight_percentage_set(50);

	return 0;
}
