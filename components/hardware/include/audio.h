#pragma once

extern float audio_volume;

void audio_init(int sample_rate);
void audio_submit(short *buf, int len);
