# ezxml: heap-buffer-overflow in `ezxml_decode()` while parsing an internal DTD

`input` makes `ezxml_parse_str()` -> `ezxml_internal_dtd()` (`src/xml.c:469`) -> `ezxml_decode()` read one byte
past the end of the caller's buffer (`src/xml.c:262`, ASan `heap-buffer-overflow`, READ of size 1). It is the
document handed to `ezxml_parse_str(buf, size)` with `buf` exactly `size` bytes, as `src/wue.c` does with a WIM's XML index.

Reproduce: `/mayhem/fuzz_xml-standalone mayhem/fuzz_xml/known-findings/internal-dtd-decode-overflow/input`.
