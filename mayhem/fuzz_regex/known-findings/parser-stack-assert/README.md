# cregex: parser aborts on `context->stack <= context->output`

`input` (`<pattern>\n<subject>`, found by libFuzzer in under a minute) makes `cregex_parse()` fail the
`assert(context->stack <= context->output)` in `push()` (`src/cregex_parse.c:19`): the parser's operand and
operator stacks share one allocation and a pattern with many quantifier/alternation tokens runs them into each other.

Reproduce: `/mayhem/fuzz_regex-standalone mayhem/fuzz_regex/known-findings/parser-stack-assert/input`.
