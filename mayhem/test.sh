#!/usr/bin/env bash
#
# mayhem/test.sh — runs the prebuilt known-answer probe (/mayhem/rufus-kat, built by build.sh with
# normal flags) over fixed inputs and compares every output line against mayhem/kat.expected.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
cd "$SRC"

emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}

KAT=/mayhem/rufus-kat
[ -x "$KAT" ] || { echo "missing $KAT: build.sh must build it" >&2; emit_ctrf rufus-kat 0 1; exit 1; }

actual="$(mktemp)"
"$KAT" mayhem/kat > "$actual" 2>/dev/null
rc=$?

[ -s mayhem/kat.expected ] || { echo "missing mayhem/kat.expected" >&2; emit_ctrf rufus-kat 0 1; exit 1; }
passed=0 failed=0
mapfile -t want < mayhem/kat.expected
mapfile -t got < "$actual"
for i in "${!want[@]}"; do
  if [ "${got[$i]-}" = "${want[$i]}" ]; then
    passed=$((passed + 1))
  else
    failed=$((failed + 1))
    echo "MISMATCH line $((i + 1)):" >&2
    echo "  want: ${want[$i]}" >&2
    echo "  got:  ${got[$i]-<missing>}" >&2
  fi
done
if [ "${#got[@]}" -ne "${#want[@]}" ]; then
  failed=$((failed + 1))
  echo "line count differs: want ${#want[@]}, got ${#got[@]}" >&2
fi
if [ "$rc" -ne 0 ]; then
  failed=$((failed + 1))
  echo "probe exited $rc" >&2
fi
rm -f "$actual"
emit_ctrf rufus-kat "$passed" "$failed"
