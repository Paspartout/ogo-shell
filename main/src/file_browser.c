#include <sys/stat.h>
#include <limits.h>     /* PATH_MAX */
#include <string.h>
#include <stdio.h>

#include <display.h>

// Application level code
#include <gbuf.h>
#include <graphics.h>
#include <OpenSans_Regular_11X12.h>
#include <tf.h>
#include <event.h>
#include <keypad.h>
#include <file_ops.h>


/* Global state. */
static struct FileBrowser {
    char cwd[PATH_MAX];
    struct Entry* cwd_entries;
    int n_entries;
    int selection;
    int scroll;
} browser;

/** Number of entries that can be displayed at once. */
static const int MAX_WIN_ENTRIES = 12;

static tf_t *font_black;
static tf_t *font_white;

static void ui_init(void) {
    font_black = tf_new(&font_OpenSans_Regular_11X12, 0x0000, 0, TF_ALIGN_CENTER);
    font_white = tf_new(&font_OpenSans_Regular_11X12, 0xFFFF, 0, TF_ALIGN_CENTER);
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
    fill_rectangle(fb, (rect_t) {.x = 0, .y = 16, .width=DISPLAY_WIDTH, .height=16}, 0xFFFF);

    // Eventually truncate path
    char *path_str;
    const int max_path_len = 40;
    char path_buf[max_path_len + 1];
    const size_t path_len = strlen(path);
    if (path_len > max_path_len) {
        fruncate_path(path_buf, path, max_path_len);
        path_str = path_buf;
    } else {
        // use path directly
        path_str = path;
    }

    tf_draw_str(fb, font_black, path_str, (point_t){.x = 3, .y = 18});

    char selection_str[16];
    snprintf(selection_str, 16, "%d/%d", selection, n_entries);
    const tf_metrics_t m = tf_get_str_metrics(font_black, selection_str);
    tf_draw_str(fb, font_black, selection_str, (point_t){.x = DISPLAY_WIDTH-m.width - 3, .y = 18});
}

static void ui_draw_browser(void) {
    // printf("sel: %d, scroll: %d\n", browser.selection, browser.scroll);

    // Draw Path
    ui_draw_pathbar(browser.cwd, browser.selection+1, browser.n_entries);

    // Draw entries
    if (browser.n_entries == 0) {
        browser.selection = 0;
        fill_rectangle(fb, (rect_t) {.x = 0, .y = 33, .width=DISPLAY_WIDTH, .height=DISPLAY_HEIGHT-33}, 0x0000);
        tf_draw_str(fb, font_white, "This directory is empty", (point_t){.x = DISPLAY_WIDTH/2-75, .y = DISPLAY_HEIGHT/2});
        display_update();
        return;
    }

    for (int i = browser.scroll; i < browser.scroll+MAX_WIN_ENTRIES; i++)
    {
		const int r = (i - browser. scroll); // window row in [0, 13]
        const int y_start = 33; // Be careful to also change MAX_WIN_ENTRIES you fool
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
        snprintf(item_str, 64, "%c - %s",
            (S_ISDIR(entry->mode)) ? 'd' : 'f',
            entry->name
            );
        tf_draw_str(fb, font_white, item_str, (point_t){.x = 2, .y = rect_y + 2 });
        
        // Draw file size
        char filesize_buf[32];
        char *suffix, *suffixes = "BKMGTPEZY";
        off_t human_size = entry->size * 10;
        for (suffix = suffixes; human_size >= 10240; suffix++)
            human_size = (human_size + 512) / 1024;
        snprintf(filesize_buf, 32, "%d.%d %c", (int)human_size/10, (int)human_size % 10, *suffix);
        const tf_metrics_t m = tf_get_str_metrics(font_white, filesize_buf);
        tf_draw_str(fb, font_white, filesize_buf, (point_t){.x = DISPLAY_WIDTH-m.width - 3, .y = rect_y + 2});
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

static void browser_init(const char* cwd) {
    strncpy(browser.cwd, cwd, strlen(cwd) + 1);
    browser.cwd_entries = NULL;
    browser.selection = 0;
    browser.scroll = 0;
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

    browser_init("/home/paspartout");
    ui_init();
    browser.n_entries = fops_list_dir(&browser.cwd_entries, browser.cwd);

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
                    const Entry* entry = &browser.cwd_entries[browser.selection];
                    if (S_ISDIR(entry->mode)) {
                        browser_cd_down(entry->name);
                    } else {
                        printf("Trying to open file: %s\n", entry->name);
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
				}
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