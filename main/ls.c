#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <display.h>

struct Entry {
	int type;
	char name[256];
};

int selection = 0; // 0 ... max


int
main(int argc, char* argv[]) {
	const char* path = ".";
	if (argc >= 2) {
		path = argv[1];
	}

	DIR *dir = opendir(path);
	if (dir == NULL) {
		perror("opendir");
		return -1;
	}

	errno = 0;
	for(struct dirent* entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
		if (errno != 0) {
			perror("readdir");
			break;
		}
		printf("%c - %s\n", entry->d_type == DT_DIR ? 'd' : 'f', entry->d_name);
	}

	closedir(dir);
	return 0;
}
