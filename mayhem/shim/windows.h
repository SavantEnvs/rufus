#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
typedef void* HANDLE;
typedef unsigned long DWORD;
#include <alloca.h>
#define _alloca alloca
#define _unlink unlink
static inline void *_aligned_malloc(size_t size, size_t align) { void *p = NULL; return posix_memalign(&p, align < sizeof(void *) ? sizeof(void *) : align, size) ? NULL : p; }
static inline void _aligned_free(void *p) { free(p); }
static inline int _open_osfhandle(intptr_t h, int flags) { (void)h; (void)flags; return -1; }
