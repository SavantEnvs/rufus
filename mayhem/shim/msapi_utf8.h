/* Linux stand-in for rufus's src/msapi_utf8.h, so that bled (and other portable rufus code) builds. */
#pragma once
#include <windows.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <limits.h>
#include <mm_malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* libbb.h typedefs these itself unless these guards are set (glibc already has them) */
#define _MODE_T_
#define _PID_T_
#define _GID_T_
#define _UID_T_

#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_CREAT O_CREAT
#define _O_TRUNC O_TRUNC
#define _O_BINARY 0
#define _S_IREAD S_IRUSR
#define _S_IWRITE S_IWUSR
#define _open open
#define _close close
#define _read read
#define _write write
#define _chmod chmod
#define _dup2 dup2
#define _mkdirU(x) mkdir((x), 0755)
#define MAX_PATH PATH_MAX

static inline int _openU(const char *f, int o, int p) { return open(f, o, p); }
static inline int PathMatchSpecA(const char *s, const char *p) { return fnmatch(p, s, 0) == 0; }
static inline int localtime_s(struct tm *r, const time_t *t) { return localtime_r(t, r) == NULL; }
static inline int SHCreateDirectoryExU(void *w, const char *p, void *s) { (void)w; (void)p; (void)s; return -1; }

/* libbb.h defines static stubs under these POSIX names; move them out of the way of glibc's declarations */
#define link bled_link
#define symlink bled_symlink
#define chown bled_chown
#define mknod bled_mknod
#define fnmatch bled_fnmatch
#define wait bled_wait
#define localtime_r bled_localtime_r

#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif
#define _snprintf_s(b, sz, cnt, ...) snprintf((b), (sz), __VA_ARGS__)
#define _SH_DENYNO 0
#define _S_IFMT S_IFMT
#define _S_IFDIR S_IFDIR
#define _S_IFCHR S_IFCHR
#define _S_IFIFO S_IFIFO
#define _S_IFREG S_IFREG
static inline int _sopen_s(int *fd, const char *p, int flags, int sh, int mode) { (void)sh; *fd = open(p, flags, mode); return *fd < 0; }
