/* Linux stand-in for the few bits of rufus's src/rufus.h that the portable sources (src/xml.c) use. */
#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "msapi_utf8.h"

#define KB 1024ULL
#define MB 1048576ULL
#define GB 1073741824ULL

#define uprintf(...) ((void)0)
#define _strdup strdup

/* MSVCRT strncpy_s(dst, dstsz, src, _TRUNCATE): copy what fits, always NUL-terminate */
static inline int strncpy_s(char *dst, size_t dstsz, const char *src, size_t count)
{
	size_t n;
	if (dst == NULL || dstsz == 0) return 22;
	if (src == NULL) { dst[0] = 0; return 22; }
	n = strlen(src);
	if (count != _TRUNCATE && count < n) n = count;
	if (n >= dstsz) n = dstsz - 1;
	memcpy(dst, src, n);
	dst[n] = 0;
	return 0;
}
