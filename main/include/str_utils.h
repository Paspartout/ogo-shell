#include <stddef.h>

/// Truncate src to dst at the back.
/// dst must be allocated with at least len bytes.
void truncate_str(char *dst, const char *src, const size_t len);

/// Truncate src to dst at the front.
/// dst must be allocated with at least len bytes.
void fruncate_str(char *dst, const char *src, const size_t len);
