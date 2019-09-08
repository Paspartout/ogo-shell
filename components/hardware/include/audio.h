#pragma once

extern float audio_volume;

typedef enum AudioOutput {
    AudioOutputSpeaker,
    AudioOutputDAC,
} AudioOutput;

/** Initialize audio driver/i2s with given sample rate and output mode. */
int audio_init(int sample_rate, const AudioOutput output);
/** Shutdown audio driver. */
int audio_shutdown(void);

/** Submit audio. */
void audio_submit(short *buf, int len);
