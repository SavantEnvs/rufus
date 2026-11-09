# cregex: a negated character class matches the string's NUL terminator

`input` is `<pattern>\n<subject>` = `[^a-z]+$` on `abc123`. In `src/cregex_vm.c` `vm_run_with_threads()`
the `REGEX_PROGRAM_OPCODE_CHARACTER_CLASS_NEGATED` case accepts `*sp == '\0'` (NUL is not in `a-z`), the thread
advances to `sp + 1` past the terminator, and the next `ASSERT_END` (`if (!*sp)` in `vm_add_thread()`) reads one
byte past the end of the subject: ASan `heap-buffer-overflow`, READ of size 1, `cregex_vm.c:81`.

Reproduce: `/mayhem/fuzz_regex-standalone mayhem/fuzz_regex/known-findings/negated-class-reads-past-nul/input`.
