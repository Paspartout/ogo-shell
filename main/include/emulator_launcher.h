#pragma once

#include <file_ops.h>

typedef struct EmulatorLauncherParam {
	Entry *entry;
	FileType rom_filetype;
	const char *cwd;
	// Filebrowser saved to NVS
	int fb_selection;
	int fb_scroll;
} EmulatorLauncherParam;

int emulator_launcher(EmulatorLauncherParam param);
