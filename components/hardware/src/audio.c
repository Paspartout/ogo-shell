#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"
#include "driver/rtc_io.h"

#include "audio.h"


#define AUDIO_IO_NEGATIVE GPIO_NUM_25
#define AUDIO_IO_POSITIVE GPIO_NUM_26
#define I2S_NUM I2S_NUM_0

float audio_volume = 1.0f;

void audio_init(int audio_sample_rate)
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
        .sample_rate = audio_sample_rate,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .use_apll = 0
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, NULL);

    audio_volume = 1.0f;
}

void audio_submit(short* buf, int len)
{
    if (audio_volume == 0.0f) {
        for (int i = 0; i < len; i += 2) {
            buf[i] = 0;
        }
    } else {
        for (int i = 0; i < len * 2; i += 2) {
            int dac0, dac1;

            /* Down mix stero to mono in sample */
            int sample = ((int)buf[i] + (int)buf[i + 1]) >> 1;

            /* Normalize */
            const float normalized = (float)sample / 0x8000;

            /* Scale */
            const int magnitude = 127 + 127;
            const float range = magnitude * normalized * audio_volume;

            /* Convert to differential output */
            if (range > 127) {
                dac1 = (range - 127);
                dac0 = 127;
            }
            else if (range < -127) {
                dac1  = (range + 127);
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

    size_t written;
    i2s_write(I2S_NUM, buf, len * 2 * sizeof(short), &written, portMAX_DELAY);
}
