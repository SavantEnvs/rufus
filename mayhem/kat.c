// Known-answer probe: drives cregex, bled and ezxml with fixed inputs and prints the results,
// so test.sh can diff the output against kat.expected.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bled.h"
#include "cregex.h"
#include "xml.h"

static void kat_regex(const char *pattern, const char *subject)
{
	const char *m[REGEX_VM_MAX_MATCHES] = { 0 };
	cregex_node_t *node = cregex_parse(pattern);
	cregex_program_t *prog = node ? cregex_compile_node(node) : NULL;
	int i, n;

	printf("regex /%s/ on \"%s\":", pattern, subject);
	if (prog == NULL) {
		printf(" compile-error\n");
	} else {
		n = cregex_program_run(prog, subject, m, 4);
		printf(" rc=%d", n);
		for (i = 0; i < 4; i++)
			if (m[i] != NULL)
				printf(" m%d@%d", i, (int)(m[i] - subject));
		printf("\n");
	}
	if (node) cregex_parse_free(node);
	if (prog) cregex_compile_free(prog);
}

static int kat_bled(const char *dir, const char *file, int type)
{
	char path[512], *src, *dst;
	size_t len;
	int64_t n;
	unsigned sum = 0;
	int64_t i;
	FILE *f;

	snprintf(path, sizeof(path), "%s/%s", dir, file);
	if ((f = fopen(path, "rb")) == NULL) { printf("bled %s: cannot open\n", file); return 1; }
	src = malloc(1 << 20);
	len = fread(src, 1, 1 << 20, f);
	fclose(f);
	dst = calloc(1, 1 << 20);
	n = bled_uncompress_from_buffer_to_buffer(src, len, dst, 1 << 20, type);
	for (i = 0; i < n; i++) sum = sum * 31 + (unsigned char)dst[i];
	printf("bled %s: %lld bytes sum=%08x head=\"%.20s\"\n", file, (long long)n, sum, n > 0 ? dst : "");
	free(src); free(dst);
	return 0;
}

static void kat_xml(const char *dir)
{
	char path[512], *buf, *out;
	size_t len;
	FILE *f;
	ezxml_t root, b;

	snprintf(path, sizeof(path), "%s/payload.xml", dir);
	if ((f = fopen(path, "rb")) == NULL) { printf("xml: cannot open\n"); return; }
	root = ezxml_parse_fp(f);
	fclose(f);
	printf("xml error=\"%s\"\n", ezxml_error(root));
	printf("xml name=%s attr=%s\n", root->name, ezxml_attr(root, "name"));
	b = ezxml_get(root, "shelf", 0, "book", 0, "");
	printf("xml shelf0/book0=\"%s\" id=%s\n", ezxml_txt(b), ezxml_attr(b, "id"));
	printf("xml shelf0/book1=\"%s\"\n", ezxml_get_val(root, "shelf", 0, "book", 1, ""));
	printf("xml shelf1/book0=\"%s\"\n", ezxml_get_val(root, "shelf", 1, "book", 0, ""));
	out = ezxml_toxml(root);
	len = strlen(out);
	printf("xml toxml %zu bytes: %s\n", len, out);
	free(out);
	ezxml_free(root);
}

int main(int argc, char **argv)
{
	const char *dir = argc > 1 ? argv[1] : "mayhem/kat";
	int failed = 0;

	kat_regex("a+b", "xxaaab");
	kat_regex("^(foo|bar)baz$", "barbaz");
	kat_regex("^(foo|bar)baz$", "foobar");
	kat_regex("[0-9]+\\.[0-9]*", "v 12.5.3");
	kat_regex("(a(b)?)+c", "xabac");
	kat_regex("a{2,3}", "caaaa");
	kat_regex("(unclosed", "x");

	bled_init(0, NULL, NULL, NULL, NULL, NULL, NULL);
	failed += kat_bled(dir, "payload.gz", BLED_COMPRESSION_GZIP);
	failed += kat_bled(dir, "payload.xz", BLED_COMPRESSION_XZ);
	failed += kat_bled(dir, "payload.bz2", BLED_COMPRESSION_BZIP2);
	failed += kat_bled(dir, "payload.lzma", BLED_COMPRESSION_LZMA);
	failed += kat_bled(dir, "payload.zip", BLED_COMPRESSION_ZIP);
	bled_exit();

	kat_xml(dir);
	return failed;
}
