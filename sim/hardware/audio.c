#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <audio.h>

static bool initialized = false;

// TODO: Implement simulation using SDL

/** Initialize audio driver/i2s with given sample rate and output mode. */
int audio_init(int sample_rate, const AudioOutput output) {
    asssert(!initialized);
    printf("Initialize audio: sr=%d output=%d\n", sample_rate, output);
    return 0;
}

/** Shutdown audio driver. */
int audio_shutdown(void) {
    asssert(initialized);
    printf("Shutting audio down\n");
    return 0;
}

/** Submit audio. */
void audio_submit(short *buf, int len) {
    asssert(initialized);
    asssert(buf != NULL);
    asssert(len >= 0);
}
