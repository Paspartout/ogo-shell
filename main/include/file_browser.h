#pragma once

typedef struct FileBrowserParam {
	const char* cwd;
} FileBrowserParam;

/** Launch file browser */
int file_browser(FileBrowserParam params);
