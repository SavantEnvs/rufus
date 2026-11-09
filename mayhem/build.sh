#!/usr/bin/env bash
#
# mayhem/build.sh — builds rufus's portable cores (cregex, bled, ezxml) on Linux and one libFuzzer
# harness (+ standalone reproducer) per core, plus the normal-flags known-answer probe used by test.sh.
# Rufus itself is a Windows application: mayhem/shim/ holds the tiny Windows-header stand-ins that let
# these sources compile unmodified.
set -euo pipefail

[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

: "${STANDALONE_FUZZ_MAIN:=/opt/mayhem/StandaloneFuzzTargetMain.c}"
cd "$SRC"
OBJ="$SRC/mayhem/build"
rm -rf "$OBJ"
mkdir -p "$OBJ/fuzz" "$OBJ/plain" "$OBJ/xml"

# ezxml's quoted #includes of rufus.h/msapi_utf8.h resolve next to the including file first, so build an
# unmodified copy of src/xml.c in a directory where the shim headers win.
cp "$SRC/src/xml.c" "$OBJ/xml/xml.c"

LIBS=(src/cregex_parse.c src/cregex_compile.c src/cregex_vm.c)
for f in src/bled/*.c; do
  case "$f" in
    src/bled/data_extract_all.c) ;;                                      # unreferenced; needs hard-link helpers rufus never defines
    src/bled/xz_dec_bcj.c|src/bled/xz_dec_lzma2.c|src/bled/xz_dec_stream.c) ;;  # #included by decompress_unxz.c
    *) LIBS+=("$f") ;;
  esac
done
LIBS+=("$OBJ/xml/xml.c")

INC=(-D_WIN32 -Wno-everything -I"$SRC/mayhem/shim" -I"$SRC/src" -I"$SRC/src/bled")

compile_one() {  # <mode> <source>
  local mode="$1" f="$2" o flags
  o="$OBJ/$mode/$(echo "${f#$SRC/}" | tr '/' '_').o"
  if [ "$mode" = fuzz ]; then
    flags="$SANITIZER_FLAGS $DEBUG_FLAGS -O1 -fsanitize=fuzzer-no-link"
    # bled stores int-returning unpackers (e.g. unpack_Z_stream) in an int64_t-returning function-pointer table;
    # UBSan's `function` check would abort every decompression before it parses a byte; busybox-style code also
    # does unaligned loads through may_alias uint32_t, which `alignment` flags in every gzip/bzip2 stream;
    # shift-base: .Z (LZW) code assembly left-shifts negative ints, which every compiler defines as the plain shift.
    case "$f" in src/bled/*) flags="$flags -fno-sanitize=function,alignment,shift-base" ;; esac
    # decompress_bunzip2.c uses busybox's `limit = hufGroup->limit - 1` offset-pointer idiom on every valid stream.
    case "$f" in src/bled/decompress_bunzip2.c) flags="$flags -fno-sanitize=bounds" ;; esac
  else
    flags="-O2 $COVERAGE_FLAGS"
  fi
  $CC $flags "${INC[@]}" -fdebug-prefix-map="$OBJ/xml/=$SRC/src/" -c "$f" -o "$o"
}
export -f compile_one
export OBJ SRC
export INC_STR="${INC[*]}"

for mode in fuzz plain; do
  printf '%s\n' "${LIBS[@]}" | xargs -P "$MAYHEM_JOBS" -I{} bash -c 'INC=($INC_STR); compile_one '"$mode"' {}'
done

# --- fuzz harnesses + standalone reproducers --------------------------------------------------------
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c "$SRC/mayhem/lsan_off.cc" -o "$OBJ/lsan_off.o"
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o "$OBJ/standalone_main.o"

for t in fuzz_regex fuzz_bled fuzz_xml; do
  $CC $SANITIZER_FLAGS $DEBUG_FLAGS -O1 "${INC[@]}" -c "$SRC/mayhem/$t.c" -o "$OBJ/$t.o"
  $CC $SANITIZER_FLAGS $DEBUG_FLAGS $LIB_FUZZING_ENGINE "$OBJ/$t.o" "$OBJ"/fuzz/*.o "$OBJ/lsan_off.o" -o "/mayhem/$t"
  $CC $SANITIZER_FLAGS $DEBUG_FLAGS "$OBJ/standalone_main.o" "$OBJ/$t.o" "$OBJ"/fuzz/*.o "$OBJ/lsan_off.o" -o "/mayhem/$t-standalone"
done

# --- known-answer probe for test.sh: normal flags, no sanitizers, no fuzzer runtime -----------------
$CC -O2 $COVERAGE_FLAGS "${INC[@]}" -c "$SRC/mayhem/kat.c" -o "$OBJ/kat.o"
$CC -O2 $COVERAGE_FLAGS "$OBJ/kat.o" "$OBJ"/plain/*.o -o /mayhem/rufus-kat
