#include <sys/stat.h>
#include <limits.h>     /* PATH_MAX */
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <display.h>

// Application level code
#include <gbuf.h>
#include <graphics.h>
#include <OpenSans_Regular_11X12.h>
#include <tf.h>
#include <event.h>
#include <keypad.h>
#include <file_ops.h>

#define COLOR_GRAY 0x4A69
#define COLOR_ORANGE 0xDB60 

/* Global state. */
static struct FileBrowser {
    char cwd[PATH_MAX];
    struct Entry* cwd_entries;
    int n_entries;
    int selection;
    int scroll;
    bool stat_enabled;
} browser;

/** Number of entries that can be displayed at once. */
static const int MAX_WIN_ENTRIES = 13;

static tf_t *font_black;
static tf_t *font_white;

static void ui_init(void) {
    font_black = tf_new(&font_OpenSans_Regular_11X12, 0x0000, 0, TF_WORDWRAP);
    font_white = tf_new(&font_OpenSans_Regular_11X12, 0xFFFF, 0, TF_WORDWRAP);
}

static void ui_free(void) {
    tf_free(font_black);
    tf_free(font_white);
}

/** Truncate src to dst at the front.
 *  dst must be allocated with at least len bytes. */
static void fruncate_path(char* dst, const char* src, const size_t len) {
        size_t i = strlen(src);
        for (size_t j = len-1; j > 2; j--) {
            dst[j] = src[i--];
        }
        dst[0] = '.';
        dst[1] = '.';
        dst[2] = '.';
        dst[len] = '\0';
}

static void ui_draw_pathbar(const char *path, int selection, int n_entries) {
    fill_rectangle(fb, (rect_t) {.x = 0, .y = 16, .width=DISPLAY_WIDTH, .height=15}, 0xFFFF);

    // Draw path
    const char *path_str;
    const int max_path_len = 40;
    char path_buf[max_path_len + 1];
    const size_t path_len = strlen(path);
    // Eventually truncate path
    if (path_len > max_path_len) {
        fruncate_path(path_buf, path, max_path_len);
        path_str = path_buf;
    } else {
        // use path directly
        path_str = path;
    }
    tf_draw_str(fb, font_black, path_str, (point_t){.x = 3, .y = 18});

    // Draw selection and number of entries
    if (n_entries > 0) {
        char selection_str[16];
        snprintf(selection_str, 16, "%d/%d", selection, n_entries);
        const tf_metrics_t m = tf_get_str_metrics(font_black, selection_str);
        tf_draw_str(fb, font_black, selection_str, (point_t){.x = DISPLAY_WIDTH-m.width - 3, .y = 18});
    }
}


// Print human readable representation of file size into dst
int sprint_human_size(char* dst, size_t strsize, off_t size) {
    char *suffix, *suffixes = "BKMGTPEZY";
    off_t human_size = size * 10;
    for (suffix = suffixes; human_size >= 10240; suffix++)
        human_size = (human_size + 512) / 1024;
    return snprintf(dst, strsize, "%d.%d %c", (int)human_size/10, (int)human_size % 10, *suffix);
}


static void ui_draw_browser(void) {
    // Draw Path
    ui_draw_pathbar(browser.cwd, browser.selection+1, browser.n_entries);

    // Show message if directory is empty
    if (browser.n_entries == 0) {
        browser.selection = 0;
        fill_rectangle(fb, (rect_t) {.x = 0, .y = 32, .width=DISPLAY_WIDTH, .height=DISPLAY_HEIGHT-32}, 0x0000);
        tf_draw_str(fb, font_white, "This directory is empty", (point_t){.x = DISPLAY_WIDTH/2-75, .y = DISPLAY_HEIGHT/2});
        display_update();
        return;
    }

    // Draw entries
    for (int i = browser.scroll; i < browser.scroll+MAX_WIN_ENTRIES; i++)
    {
		const int r = (i - browser. scroll); // window row in [0, 13]
        const int y_start = 32; // Be careful to also change MAX_WIN_ENTRIES you fool
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
        const Entry* entry = &browser.cwd_entries[i];
        char item_str[64];
        char fname_buf[41];
        char *filename;
        if (strlen(entry->name) > 40) {
            fruncate_path(fname_buf, entry->name, 40);
            filename = fname_buf;
        } else {
            filename = entry->name;
        }
        snprintf(item_str, 64, "%c - %s",
            (S_ISDIR(entry->mode)) ? 'd' : 'f',
            filename
            );
        tf_draw_str(fb, font_white, item_str, (point_t){.x = 2, .y = rect_y + 2 });
        
        // Draw file size
        if (browser.stat_enabled) {
            char filesize_buf[32];
            sprint_human_size(filesize_buf, 32, entry->size);
            const tf_metrics_t m = tf_get_str_metrics(font_white, filesize_buf);
            tf_draw_str(fb, font_white, filesize_buf, (point_t){.x = DISPLAY_WIDTH-m.width - 3, .y = rect_y + 2});
        }
    }
    
    display_update();
}


static void browser_scroll(int amount) {
    if (amount == 0) {
        return;
    }
    if (amount > 0) {
        for (int i = 0; i < amount; i++) {
            if (++browser.selection >= browser.n_entries-1) {
                browser.selection = browser.n_entries-1;
            }
            if (browser.selection - browser.scroll > (MAX_WIN_ENTRIES-1)) {
                browser.scroll++;
            }
        }
    } else {
        amount = -amount;
        for (int i = 0; i < amount; i++)
        {
            if(--browser.selection < 0) {
                browser.selection = 0;
            }
            if (browser.selection - browser.scroll < 0) {
                browser.scroll--;
            }
        }
    }
}


// TODO: printf like behaviour would be nice for detailed error messages
static void ui_message_error(const char *msg) {
    // TODO: Display error on display
    const int ypos = 104;
    fill_rectangle(fb, (rect_t){.x=0, .y=ypos, .width=DISPLAY_WIDTH, .height=16}, 0xF800);
    tf_draw_str(fb, font_white, msg, (point_t) {.x = 3, .y = ypos+3});
    fprintf(stderr, "error: %s\n", msg);
    display_update();

    event_t event;
    wait_event(&event);
    while(event.type != EVENT_TYPE_KEYPAD) {};

}

static void ui_draw_details(Entry *entry, const char* cwd) {
    // Try to retrieve file stats
    if (fops_stat_entry(entry, cwd) == -1) {
        ui_message_error("Could not get file status");
        return;
    }

    char str_buf[300];
    char filesize_buf[32];
    fill_rectangle(fb, (rect_t) {.x = 0, .y = 32, .width=DISPLAY_WIDTH, .height=DISPLAY_HEIGHT-32}, COLOR_GRAY);
    const int line_height = 16;
    int y = 34;
    tf_draw_str(fb, font_white, "Details", (point_t) {.x = 3, .y = y});
    y += line_height;
    // Full file name
    // TODO: Figure out wordwrapping or scrolling for long filenames
    snprintf(str_buf, 300, "Name: %s", entry->name);
    tf_draw_str(fb, font_white, str_buf, (point_t) {.x = 3, .y = y});
    y += line_height * 2;
    // File size
    sprint_human_size(filesize_buf, 32, entry->size);
    snprintf(str_buf, 256, "Size: %s(%ld Bytes)", filesize_buf, entry->size);
    tf_draw_str(fb, font_white, str_buf, (point_t) {.x = 3, .y = y});
    y += line_height;
    // Modification time
    sprint_human_size(filesize_buf, 32, entry->size);
    ctime_r(&entry->mtime, filesize_buf);
    snprintf(str_buf, 256, "Modification time: %s", filesize_buf);
    tf_draw_str(fb, font_white, str_buf, (point_t) {.x = 3, .y = y});
    y += line_height;
    // Permissions?
    mode_t permissions = entry->mode & 0777;
    snprintf(str_buf, 256, "Permissions: %o", permissions);
    tf_draw_str(fb, font_white, str_buf, (point_t) {.x = 3, .y = y});
    // TODO Later: filetype using libmagic?

    display_update();
    event_t event;
    for(;;) {
        wait_event(&event);
        if (event.type == EVENT_TYPE_KEYPAD && event.keypad.pressed)
            break;
    }

    ui_draw_browser();
}

static void browser_init(const char* cwd) {
    strncpy(browser.cwd, cwd, strlen(cwd) + 1);
    browser.cwd_entries = NULL;
    browser.selection = 0;
    browser.scroll = 0;
    browser.stat_enabled = false;
}

static int browser_cd(const char* new_cwd) {
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

    ui_draw_browser();
    return 0;
}

static int path_cd_up(char *cwd, char *new_cwd) {
    if (strlen(cwd) <= 1 && cwd[0] == '/') {
        // Cant go up anymore
        return -1;
    }

    // Remove upmost directory
    bool copy = false;
    int len = 0;
    for (ssize_t i = strlen(cwd); i >= 0; i--) {
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

static int browser_cd_down(const char* dir) {
    char new_cwd[PATH_MAX];
    const char* sep = !strncmp(browser.cwd, "/", 2) ? "" : "/"; // Don't append / if at /
    snprintf(new_cwd, PATH_MAX, "%s%s%s", browser.cwd, sep, dir);
    return browser_cd(new_cwd);
}

static int browser_cd_up(void) {
    char new_cwd[PATH_MAX];
    if (path_cd_up(browser.cwd, new_cwd) < 0) {
        ui_message_error("Can't go up anymore! Already at topmost directory.");
        return -1;
    }

    return browser_cd(new_cwd);
}


int file_browser(void) {
	bool quit = false;
	event_t event;

    browser_init("/sdcard");
    ui_init();
    browser.n_entries = fops_list_dir(&browser.cwd_entries, browser.cwd);
    if (browser.n_entries < 0) {
        browser.n_entries = 0;
    }

    ui_draw_browser();

	while(!quit) {
		// Handle inputs
		if(wait_event(&event) < 0) {
            continue;
        }
		switch(event.type) {
			case EVENT_TYPE_KEYPAD:
			{
				switch(event.keypad.pressed) {
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
                    Entry* entry = &browser.cwd_entries[browser.selection];
                    if (S_ISDIR(entry->mode)) {
                        browser_cd_down(entry->name);
                    } else {
                        printf("Trying to open file: %s\n", entry->name);
                        ui_draw_details(entry, browser.cwd);
                        // TODO: File handlers
                    }
                }
					break;
				case KEYPAD_B:
                    browser_cd_up();
					break;
				case KEYPAD_MENU:
                    // TODO: Help Window + Quit Option
					quit = true;
					break;
				case KEYPAD_START:
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
            }
				break;
			default:
				printf("OTHER!\n");
				break;
        case EVENT_TYPE_QUIT:
            quit = true;
            break;
        case EVENT_TYPE_UPDATE:
            display_update();
            break;
		}
	}
    ui_free();
    fops_free_entries(&browser.cwd_entries, browser.n_entries);

    return 0;
}