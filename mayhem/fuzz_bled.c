// Fuzzes rufus's bled decompressors (src/bled): byte 0 selects the format, the rest is the compressed stream.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "bled.h"

#define DST_SIZE (64 * 1024)

static void quiet_printf(const char *format, ...) { (void)format; }

int LLVMFuzzerInitialize(int *argc, char ***argv)
{
	(void)argc; (void)argv;
	bled_init(0, quiet_printf, NULL, NULL, NULL, NULL, NULL);
	return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char *src, *dst;
	int type;

	if (size < 2)
		return -1;
	type = data[0] % BLED_COMPRESSION_MAX;
	src = malloc(size - 1);
	dst = malloc(DST_SIZE);
	memcpy(src, data + 1, size - 1);
	bled_uncompress_from_buffer_to_buffer(src, size - 1, dst, DST_SIZE, type);
	free(dst);
	free(src);
	return 0;
}
