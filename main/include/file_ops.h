#pragma once

#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>

/** Information associated to each directory entry */
typedef struct Entry {
    char *name;  /** File name */
    off_t size;  /** File size in bytes? */
    mode_t mode; /** Access permissions? */
} Entry;

/** List all entries of given cwd. */
int fops_list_dir(Entry **entries, const char *cwd);
/** List all entries of given cwd. */
void fops_free_entries(Entry **entries, int n_entires);