#include <limits.h> /* PATH_MAX */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <file_browser.h>

#include <display.h>

// Application level code
#include <OpenSans_Regular_11X12.h>
#include <event.h>
#include <file_ops.h>
#include <gbuf.h>
#include <graphics.h>
#include <keypad.h>
#include <tf.h>
#include <ui.h>
#include <str_utils.h>
#include <settings.h>

#include <audio_player.h>
#include <image_viewer.h>
#include <emulator_launcher.h>
#include <icon_img.c>

#ifndef SIM
#include <esp_system.h>
#endif

/* Global state. */
static struct FileBrowser {
	char cwd[PATH_MAX];
	struct Entry *cwd_entries;
	int n_entries;
	int selection;
	int scroll;
	bool stat_enabled;
} browser;

/** Number of entries that can be displayed at once. */
static const int MAX_WIN_ENTRIES = 13;

static void draw_pathbar(const char *path, int selection, int n_entries)
{
	char selection_str[16];
	char *right = NULL;
	if (n_entries > 0) {
		snprintf(selection_str, 16, "%d/%d", selection, n_entries);
		right = selection_str;
	}
	ui_draw_pathbar(path, right, true);
}

// Print human readable representation of file size into dst
int sprint_human_size(char *dst, size_t strsize, off_t size)
{
	char *suffix, *suffixes = "BKMGTPEZY";
	off_t human_size = size * 10;
	for (suffix = suffixes; human_size >= 10240; suffix++)
		human_size = (human_size + 512) / 1024;
	return snprintf(dst, strsize, "%d.%d %c", (int)human_size / 10, (int)human_size % 10, *suffix);
}

static void ui_draw_browser(void)
{
	// Draw Path
	draw_pathbar(browser.cwd, browser.selection + 1, browser.n_entries);

	// Show message if directory is empty
	if (browser.n_entries == 0) {
		browser.selection = 0;
		fill_rectangle(fb, (rect_t){.x = 0, .y = 32, .width = DISPLAY_WIDTH, .height = DISPLAY_HEIGHT - 32}, 0x0000);
		tf_draw_str(fb, ui_font_white, "This directory is empty", (point_t){.x = DISPLAY_WIDTH / 2 - 75, .y = DISPLAY_HEIGHT / 2});
		display_update();
		return;
	}

	// Display icon image
	gbuf_t img = {.width = (uint16_t)icon_img.width,
		      .height = (uint16_t)icon_img.height,
		      .bytes_per_pixel = 2,
		      .data = (uint8_t *)&icon_img.pixel_data,
		      .big_endian = false};

	// Draw entries
	for (int i = browser.scroll; i < browser.scroll + MAX_WIN_ENTRIES; i++) {
		const int r = (i - browser.scroll); // window row in [0, 13]
		const int y_start = 32;		    // Be careful to also change MAX_WIN_ENTRIES you fool
		const int line_height = 16;
		const int rect_y = y_start + r * 16;

		if (i > browser.n_entries - 1) {
			fill_rectangle(fb, (rect_t){.x = 0, .y = rect_y, .width = DISPLAY_WIDTH, .height = line_height}, 0x0000);
			continue;
		}

		// Draw background depending on selection
		const uint16_t bg_color = i == browser.selection ? 0xDB60 : 0x4A69;
		fill_rectangle(fb, (rect_t){.x = 0, .y = rect_y, .width = DISPLAY_WIDTH, .height = line_height}, bg_color);

		// Draw filename
		const Entry *entry = &browser.cwd_entries[i];
		char fname_buf[41];
		char *filename;
		if (strlen(entry->name) > 40) {
			fruncate_str(fname_buf, entry->name, 40);
			filename = fname_buf;
		} else {
			filename = entry->name;
		}

		rect_t rt;
		rt.x = 0;
		rt.width = 11;
		rt.height = 12;
		if (S_ISDIR(entry->mode)) rt.y = 12; else rt.y = 0;
		blit(fb, (rect_t){.x = 2, .y = rect_y + 2, .width = 11, .height = 12}, &img, rt);
		tf_draw_str(fb, ui_font_white, filename, (point_t){.x = 15, .y = rect_y + 4});

		// Draw file size
		if (browser.stat_enabled) {
			char filesize_buf[32];
			sprint_human_size(filesize_buf, 32, entry->size);
			const tf_metrics_t m = tf_get_str_metrics(ui_font_white, filesize_buf);
			tf_draw_str(fb, ui_font_white, filesize_buf, (point_t){.x = DISPLAY_WIDTH - m.width - 3, .y = rect_y + 2});
		}
	}

	display_update();
}

static void browser_scroll(int amount)
{
	if (amount == 0) {
		return;
	}
	if (amount > 0) {
		for (int i = 0; i < amount; i++) {
			if (++browser.selection >= browser.n_entries - 1) {
				browser.selection = browser.n_entries - 1;
			}
			if (browser.selection - browser.scroll > (MAX_WIN_ENTRIES - 1)) {
				browser.scroll++;
			}
		}
	} else {
		amount = -amount;
		for (int i = 0; i < amount; i++) {
			if (--browser.selection < 0) {
				browser.selection = 0;
			}
			if (browser.selection - browser.scroll < 0) {
				browser.scroll--;
			}
		}
	}
}

static void ui_draw_details(Entry *entry, const char *cwd)
{
	// Try to retrieve file stats
	if (fops_stat_entry(entry, cwd) == -1) {
		ui_message_error("Could not get file status");
		return;
	}

	char str_buf[300];
	char filesize_buf[32];
	fill_rectangle(fb, (rect_t){.x = 0, .y = 32, .width = DISPLAY_WIDTH, .height = DISPLAY_HEIGHT - 32}, COLOR_GRAY);
	const int line_height = 16;
	short y = 34;
	tf_draw_str(fb, ui_font_white, "Details", (point_t){.x = 3, .y = y});
	y += line_height * 2;
	// Full file name
	// TODO: Figure out wordwrapping or scrolling for long filenames
	snprintf(str_buf, 300, "Name: %s", entry->name);
	tf_draw_str(fb, ui_font_white, str_buf, (point_t){.x = 3, .y = y});
	y += line_height * 2;
	// File size
	sprint_human_size(filesize_buf, 32, entry->size);
	snprintf(str_buf, 256, "Size: %s(%ld Bytes)", filesize_buf, entry->size);
	tf_draw_str(fb, ui_font_white, str_buf, (point_t){.x = 3, .y = y});
	y += line_height;
	// Modification time
	sprint_human_size(filesize_buf, 32, entry->size);
	ctime_r(&entry->mtime, filesize_buf);
	snprintf(str_buf, 256, "Modification time: %s", filesize_buf);
	tf_draw_str(fb, ui_font_white, str_buf, (point_t){.x = 3, .y = y});
	y += line_height;
	// Permissions
	const mode_t permissions = entry->mode & 0777;
	snprintf(str_buf, 256, "Permissions: %o", permissions);
	tf_draw_str(fb, ui_font_white, str_buf, (point_t){.x = 3, .y = y});
	// TODO Later: filetype using libmagic?

	display_update();
	event_t event;
	for (;;) {
		wait_event(&event);
		if (event.type == EVENT_TYPE_KEYPAD && event.keypad.pressed)
			break;
	}
}

static void browser_init(const char *cwd)
{
	strncpy(browser.cwd, cwd, strlen(cwd) + 1);
	browser.cwd_entries = NULL;
	browser.selection = 0;
	browser.scroll = 0;
	browser.stat_enabled = false;
}

static int browser_cd(const char *new_cwd)
{
	Entry *new_entries;

	int n_entries = fops_list_dir(&new_entries, new_cwd);
	if (n_entries < 0) {
		ui_message_error("Can't change directory");
		return -1;
	}

	// Apply cd after successfully listing new directory
	fops_free_entries(&browser.cwd_entries, browser.n_entries);
	browser.n_entries = n_entries;
	browser.selection = 0;
	browser.scroll = 0;
	browser.cwd_entries = new_entries;
	strncpy(browser.cwd, new_cwd, PATH_MAX);

	if (browser.stat_enabled)
		fops_stat_entries(browser.cwd_entries, browser.n_entries, browser.cwd);

	return 0;
}

static int path_cd_up(char *cwd, char *new_cwd)
{
	if (strlen(cwd) <= 1 && cwd[0] == '/') {
		// Cant go up anymore
		return -1;
	}

	// Remove upmost directory
	bool copy = false;
	int len = 0;
	for (ssize_t i = (ssize_t)strlen(cwd); i >= 0; i--) {
		if (cwd[i] == '/')
			copy = true;
		if (copy) {
			new_cwd[i] = cwd[i];
			len++;
		}
	}
	// remove trailing slash
	if (len > 1 && new_cwd[len - 1] == '/')
		len--;
	new_cwd[len] = '\0';

	return 0;
}

static int browser_cd_down(const char *dir)
{
	char new_cwd[PATH_MAX];
	const char *sep = !strncmp(browser.cwd, "/", 2) ? "" : "/"; // Don't append / if at /
	snprintf(new_cwd, PATH_MAX, "%s%s%s", browser.cwd, sep, dir);
	return browser_cd(new_cwd);
}

static int browser_cd_up(void)
{
	char new_cwd[PATH_MAX];
	if (path_cd_up(browser.cwd, new_cwd) < 0) {
		ui_message_error("Can't go up anymore! Already at topmost directory.");
		return -1;
	}

	return browser_cd(new_cwd);
}

static void open_file(Entry *entry)
{
	// TODO: Proper File handlers
	const FileType ftype = fops_determine_filetype(entry);
	if (ftype == FileTypeMP3 || ftype == FileTypeOGG || ftype == FileTypeMOD || ftype == FileTypeWAV || ftype == FileTypeFLAC ||
	    ftype == FileTypeGME) {
		audio_player((AudioPlayerParam){browser.cwd_entries, browser.n_entries, browser.selection, browser.cwd, true});
	} else if (ftype == FileTypeJPEG || ftype == FileTypePNG || ftype == FileTypeBMP || ftype == FileTypeGIF) {
		image_viewer((ImageViewerParams){browser.cwd_entries, browser.n_entries, browser.selection, browser.cwd});
	} else if (ftype == FileTypeGB || ftype == FileTypeGBC || ftype == FileTypeNES || ftype == FileTypeGG || ftype == FileTypeCOL ||
		   ftype == FileTypeSMS) {
		emulator_launcher((EmulatorLauncherParam){.entry = &browser.cwd_entries[browser.selection],
							  .rom_filetype = ftype,
							  .cwd = browser.cwd,
							  .fb_selection = browser.selection,
							  .fb_scroll = browser.scroll});
	} else {
		ui_draw_details(entry, browser.cwd);
	}
}

static int browser_load_settings(void)
{
#ifndef SIM
	esp_reset_reason_t reason = esp_reset_reason();
	fprintf(stderr, "reset reason: %d\n", reason);
	if (reason != ESP_RST_SW) {
		settings_save(SettingLastSelection, -1);
		return -2;
	}
#endif

	char last_path[PATH_MAX];
	int32_t last_selection, last_scroll;

	// Try to load last selection and invaildate it afterwards
	if (settings_load(SettingLastSelection, &last_selection)) {
		return -1;
	}
	if (last_selection < 0) {
		return -1;
	}
	if (settings_load_str(SettingLastPath, last_path, PATH_MAX) != 0) {
		return -1;
	}
	if (settings_load(SettingLastScroll, &last_scroll) != 0) {
		return -1;
	}

	printf("Loading last path %s with selection %d and scroll %d\n", last_path, last_selection, last_scroll);
	if (browser_cd(last_path) != 0) {
		settings_save(SettingLastSelection, -1);
		return -1;
	}
	strncpy(browser.cwd, last_path, PATH_MAX);
	if (last_selection < browser.n_entries) {
		browser.selection = last_selection;
		if (last_scroll < browser.n_entries) {
			browser.scroll = last_scroll;
		}
	}

	settings_save(SettingLastSelection, -1);

	return 0;
}

int file_browser(FileBrowserParam params)
{
	bool quit = false;
	event_t event;

	browser_init(params.cwd);
	browser_load_settings();

	browser.n_entries = fops_list_dir(&browser.cwd_entries, browser.cwd);
	if (browser.n_entries < 0) {
		browser.n_entries = 0;
	}

	ui_draw_browser();

	while (!quit) {
		// Handle inputs
		if (wait_event(&event) < 0) {
			continue;
		}
		switch (event.type) {
		case EVENT_TYPE_KEYPAD: {
			switch (event.keypad.pressed) {
			case KEYPAD_UP:
				browser_scroll(-1);
				ui_draw_browser();
				break;
			case KEYPAD_DOWN:
				browser_scroll(1);
				ui_draw_browser();
				break;
			case KEYPAD_RIGHT:
				browser_scroll(MAX_WIN_ENTRIES);
				ui_draw_browser();
				break;
			case KEYPAD_LEFT:
				browser_scroll(-MAX_WIN_ENTRIES);
				ui_draw_browser();
				break;
			case KEYPAD_A: {
				if (browser.n_entries <= 0) {
					continue;
				}
				Entry *entry = &browser.cwd_entries[browser.selection];
				if (S_ISDIR(entry->mode)) {
					browser_cd_down(entry->name);
				} else {
					open_file(entry);
				}
				ui_draw_browser();
			} break;
			case KEYPAD_B:
				browser_cd_up();
				ui_draw_browser();
				break;
			case KEYPAD_MENU:
				// TODO: Help Window + Quit Option
				quit = true;
				break;
			case KEYPAD_START:
				if (browser.n_entries <= 0) {
					continue;
				}
				ui_draw_details(&browser.cwd_entries[browser.selection], browser.cwd);
				break;
			case KEYPAD_SELECT:
				// Toggle detailed information mode
				browser.stat_enabled = !browser.stat_enabled;
				if (browser.stat_enabled) {
					fops_stat_entries(browser.cwd_entries, browser.n_entries, browser.cwd);
				}
				ui_draw_browser();
				break;
			} // switch(event.keypad.pressed)
		} break;
		case EVENT_TYPE_QUIT:
			quit = true;
			break;
		case EVENT_TYPE_UPDATE:
			display_update();
			break;
		default:
			printf("Other event detected\n");
			break;
		} // switch(event.type)
	}
	fops_free_entries(&browser.cwd_entries, browser.n_entries);

	return 0;
}
