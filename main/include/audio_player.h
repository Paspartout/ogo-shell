#pragma once

#include <file_ops.h>

/** Start the audio player at the file entry at index. */
int audio_player(Entry* entries, int index, const char *cwd);