#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>     /* chdir(), getcwd(), read(), close(), ... */

#include <file_ops.h>

// TODO: Add different comparisons and let user decide how to sort listing
/* Comparison used to sort listing entries. */
static int
entry_cmp(const void *a, const void *b)
{
    int isdir1, isdir2, cmpdir;
    const Entry *r1 = a;
    const Entry *r2 = b;
    isdir1 = S_ISDIR(r1->mode);
    isdir2 = S_ISDIR(r2->mode);
    cmpdir = isdir2 - isdir1;
    return cmpdir ? cmpdir : strcoll(r1->name, r2->name);
}

/** List all entries of given cwd. */
int
fops_list_dir(Entry **entriesp, const char *cwd) {
    DIR *dp;
    struct dirent *ep;
    struct stat statbuf;
    Entry *entries;
    int i, n;
    char path[PATH_MAX];

    if(!(dp = opendir(cwd))) return -1;
    n = -2; /* We don't want the entries "." and "..". */
    while (readdir(dp)) n++;
    if (n == 0) {
        closedir(dp);
        return 0;
    }
    rewinddir(dp);
    entries = malloc(n * sizeof(*entries));
    i = 0;
    while ((ep = readdir(dp))) {
        // TODO: Error handling
        /* Skip "." and ".." entries */
        if (!strncmp(ep->d_name, ".", 2) || !strncmp(ep->d_name, "..", 3))
            continue;

        const size_t fname_size = strlen(ep->d_name) + 1;
        entries[i].name = malloc(fname_size);
        strncpy(entries[i].name, ep->d_name, fname_size);

        snprintf(path, PATH_MAX, "%s/%s", cwd, ep->d_name);
        if (stat(path, &statbuf) != 0) {
            // TODO: How to signal error?
            perror("stat");
            continue;
        }
        entries[i].size = statbuf.st_size;
        entries[i].mode = statbuf.st_mode;
        i++;
    }
    n = i; /* Ignore unused space in array caused by filters. */
    qsort(entries, n, sizeof (*entries), entry_cmp);
    closedir(dp);
    *entriesp = entries;
    return n;   
}

/** Free memroy from given entries. */
void fops_free_entries(Entry **entries, int n_entires) {
    int i;

    for (i = 0; i < n_entires; i++)
        free((*entries)[i].name);
    if (n_entires > 0)
        free(*entries);
    *entries = NULL;
}
