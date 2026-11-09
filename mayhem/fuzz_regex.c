// Fuzzes rufus's cregex engine (src/cregex_*.c): "<pattern>\n<subject>" is parsed, compiled and run.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cregex.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char *buf, *nl;
	const char *subject, *matches[REGEX_VM_MAX_MATCHES];
	cregex_node_t *node;
	cregex_program_t *program;

	if (size < 1 || size > 4096)
		return -1;
	buf = malloc(size + 1);
	memcpy(buf, data, size);
	buf[size] = 0;
	subject = buf;
	if ((nl = memchr(buf, '\n', size)) != NULL) {
		*nl = 0;
		subject = nl + 1;
	}
	node = cregex_parse(buf);
	if (node != NULL) {
		program = cregex_compile_node(node);
		cregex_parse_free(node);
		if (program != NULL) {
			memset(matches, 0, sizeof(matches));
			cregex_program_run(program, subject, matches, REGEX_VM_MAX_MATCHES);
			cregex_compile_free(program);
		}
	}
	free(buf);
	return 0;
}
