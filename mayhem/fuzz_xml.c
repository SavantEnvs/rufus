// Fuzzes rufus's ezxml parser (src/xml.c) the way src/wue.c uses it: ezxml_parse_str() on a caller-owned
// buffer (a WIM's XML index), ezxml_get_val() lookups, and for well-formed documents a walk + serialization.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "xml.h"

static void walk(ezxml_t node, int depth)
{
	for (; node != NULL && depth < 64; node = node->ordered) {
		(void)ezxml_attr(node, "id");
		(void)ezxml_txt(node);
		(void)ezxml_child(node, "a");
		walk(node->child, depth + 1);
	}
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char *buf, *out;
	ezxml_t root;

	if (size < 1 || size > 65536)
		return -1;
	buf = malloc(size);
	memcpy(buf, data, size);
	root = ezxml_parse_str(buf, size);
	if (root != NULL) {
		(void)ezxml_get_val(root, "IMAGE", 1, "WINDOWS", 0, "LANGUAGES", 0, "LANGUAGE", -1);
		(void)ezxml_get_val(root, "IMAGE", 0, "WINDOWS", 0, "VERSION", 0, "MAJOR", -1);
		if (*ezxml_error(root) == 0) {
			walk(root, 0);
			(void)ezxml_pi(root, "xml");
			out = ezxml_toxml(root);
			free(out);
		}
		ezxml_free(root);
	}
	free(buf);
	return 0;
}
