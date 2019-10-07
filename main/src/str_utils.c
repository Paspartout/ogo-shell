#include <str_utils.h>

#include <string.h>

/// Truncate src to dst at the back.
/// dst must be allocated with at least len bytes.
void truncate_str(char *dst, const char *src, const size_t len)
{
	memcpy(dst, src, len - 4);
	dst[len - 4] = '.';
	dst[len - 3] = '.';
	dst[len - 2] = '.';
	dst[len - 1] = '\0';
}

/// Truncate src to dst at the front.
/// dst must be allocated with at least len bytes.
void fruncate_str(char *dst, const char *src, const size_t len)
{
	size_t i = strlen(src);
	for (size_t j = len - 1; j >= 3; j--) {
		dst[j] = src[i--];
	}
	dst[0] = '.';
	dst[1] = '.';
	dst[2] = '.';
	dst[len - 1] = '\0';
}
