#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>
#include <driver/rtc_io.h>
#include <driver/dac.h>

#include <stdio.h>

#include "audio.h"

#define AUDIO_IO_NEGATIVE GPIO_NUM_25
#define AUDIO_IO_POSITIVE GPIO_NUM_26
#define I2S_NUM I2S_NUM_0

float audio_volume = 0.5f;

static bool initialized = false;
static AudioOutput chosen_output = AudioOutputSpeaker;

static int shutdown_speaker() {
	esp_err_t error;
	const char *error_message = "Could not shutdown dac or amplifier amp: %s\n";
	if ((error = i2s_set_dac_mode(I2S_DAC_CHANNEL_DISABLE)) != ESP_OK) {
		fprintf(stderr, error_message, esp_err_to_name(error));
		return -1;
	}
	if ((error = gpio_set_direction(AUDIO_IO_NEGATIVE, GPIO_MODE_OUTPUT)) != ESP_OK) {
		fprintf(stderr, error_message, esp_err_to_name(error));
		return -1;
	}
	if ((error = gpio_set_direction(AUDIO_IO_POSITIVE, GPIO_MODE_DISABLE)) != ESP_OK) {
		fprintf(stderr, error_message, esp_err_to_name(error));
		return -1;
	}
	if ((error = gpio_set_level(AUDIO_IO_NEGATIVE, 0)) != ESP_OK) {
		fprintf(stderr, error_message, esp_err_to_name(error));
		return -1;
	}
	return 0;
}

int audio_init(int audio_sample_rate, const AudioOutput output)
{
	if (initialized) {
		fprintf(stderr, "Audio already initialized!\n");
		return -1;
	}
	const i2s_mode_t mode_dac = I2S_MODE_MASTER | I2S_MODE_TX;
	const i2s_mode_t mode_speaker = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN;
	const i2s_comm_format_t commfmt_dac = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB;
	const i2s_comm_format_t commfmt_speaker = I2S_COMM_FORMAT_I2S_MSB;

	i2s_config_t i2s_config = {
				   .mode = output == AudioOutputSpeaker ? mode_speaker : mode_dac,
				   .sample_rate = audio_sample_rate,
				   .bits_per_sample = 16,
				   .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
				   .communication_format = output == AudioOutputSpeaker ? commfmt_speaker : commfmt_dac,
				   .dma_buf_count = 6,
				   .dma_buf_len = 512,
				   .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
				   .use_apll = output == AudioOutputDAC ? true : false};
	const i2s_pin_config_t dac_pin_config = {
		.bck_io_num = 4,
		.ws_io_num = 12,
		.data_out_num = 15,
		.data_in_num = -1                                                       //Not used
	};

	esp_err_t error;
	if ((error = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL)) != ESP_OK) {
		// TODO: Need some other system wide error handling
		fprintf(stderr, "Could not install i2s driver: %s\n", esp_err_to_name(error));
		return -1;
	}
	if ((error = i2s_set_pin(I2S_NUM, output == AudioOutputDAC ? &dac_pin_config : NULL)) != ESP_OK) {
		fprintf(stderr, "Could not set i2s pin: %s\n", esp_err_to_name(error));
		return -1;
	}

	// Disable interal amplifier when using DAC
	if (output == AudioOutputDAC) {
		shutdown_speaker();
	}

	chosen_output = output;
	initialized = true;
	audio_volume = 1.0f;
	return 0;
}

int audio_shutdown(void) {
	if (!initialized) {
		fprintf(stderr, "Audio was not initialized!\n");
		return -1;
	}

	esp_err_t error;
	if ((error = i2s_driver_uninstall(I2S_NUM)) != ESP_OK) {
		fprintf(stderr, "Could uninstall i2s driver: %s\n", esp_err_to_name(error));
		return -1;
	}

	shutdown_speaker();
	initialized = false;

	return 0;
}

/** Convert the given buffer to the proper format for internal dac. */
static void convert_internal_dac(short* buf, const int n_frames) {
	for (int i = 0; i < n_frames * 2; i += 2) {
		int dac0, dac1;

		/* Down mix stero to mono in sample */
		int sample = ((int)buf[i] + (int)buf[i + 1]) >> 1;

		/* Normalize */
		const float normalized = (float)sample / 0x8000;

		/* Scale */
		const int magnitude = 127 + 127;
		const float range = magnitude * normalized * audio_volume;

		/* Convert to differential output. */
		if (range > 127) {
			dac1 = (range - 127);
			dac0 = 127;
		} else if (range < -127) {
			dac1 = (range + 127);
			dac0 = -127;
		} else {
			dac1 = 0;
			dac0 = range;
		}

		dac0 += 0x80;
		dac1 = 0x80 - dac1;

		dac0 <<= 8;
		dac1 <<= 8;

		buf[i] = (short)dac1;
		buf[i + 1] = (short)dac0;
	}
}

/** Apply volume for external dac buffer */
static void apply_volume(short *buf, const int n_frames) {
	for (int i = 0; i < n_frames * 2; ++i)
	{
		// Apply volume
		int sample = buf[i] * audio_volume;

		// clamp sample to short values
		// TODO: Should not be needed if audio_volume <= 1.0f
		if (sample > 32767)
			sample = 32767;
		else if (sample < -32768)
			sample = -32767;

		buf[i] = (short)sample;
	}
}

void audio_submit(short *buf, int n_frames)
{
	if (!initialized) {
		fprintf(stderr, "audio not yet initialized");
		return;
	}
	if (audio_volume == 0.0f) {
		for (int i = 0; i < n_frames; i += 2) {
			buf[i] = 0;
			buf[i+1] = 0;
		}
	}

	if (chosen_output == AudioOutputSpeaker) {
		convert_internal_dac(buf, n_frames);
	} else {
		apply_volume(buf, n_frames);
	}

	const size_t to_write = 2 * n_frames * sizeof(short);
	size_t written;
	i2s_write(I2S_NUM, buf, to_write, &written, portMAX_DELAY);
	if (written != to_write) {
		fprintf(stderr, "error submitting data to i2s");
	}
}
