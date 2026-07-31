#!/usr/bin/env bash
#
# CLI contract tests for hopfieldann.
#
# Drives the built binary with piped stdin and asserts exit codes plus
# key output lines, covering: usage, error paths, noise-level validation,
# pattern-index validation, R/Enter repeat semantics, L reload, and mode 2.
#
# Usage: cli_contract_test.sh <path-to-hopfieldann-binary>
# Run from the project root so data/ and bin/ paths are relative.

set -u

BIN="$1"

PASS=0
FAIL=0
FAILED=()

TMP="$(mktemp -d)"
OUT="$TMP/out.txt"
trap 'rm -rf "$TMP"' EXIT

# check <name> <expected_exit> <input> <binary_args> <must_match> <must_not_match> <count_pat> <count_min> <count_max>
#   count_max == -1 means no upper bound
check() {
    local name="$1" exp_exit="$2" input="$3" args="$4"
    local must="$5" mustnot="$6" countpat="$7" countmin="$8" countmax="$9"

    printf '%b' "$input" | "$BIN" $args >"$OUT" 2>&1
    local got_exit=$?

    local ok=1 reason=""
    if [ "$got_exit" -ne "$exp_exit" ]; then
        ok=0; reason="exit $got_exit (expected $exp_exit)"
    fi
    if [ "$ok" = 1 ] && [ -n "$must" ] && ! grep -q "$must" "$OUT"; then
        ok=0; reason="missing output '$must'"
    fi
    if [ "$ok" = 1 ] && [ -n "$mustnot" ] && grep -q "$mustnot" "$OUT"; then
        ok=0; reason="unexpected output '$mustnot'"
    fi
    if [ "$ok" = 1 ] && [ -n "$countpat" ]; then
        local n
        n="$(grep -c "$countpat" "$OUT" 2>/dev/null || true)"
        if [ "$n" -lt "$countmin" ] || { [ "$countmax" -ge 0 ] && [ "$n" -gt "$countmax" ]; }; then
            ok=0; reason="count of '$countpat' is $n (expected $countmin..$countmax)"
        fi
    fi

    if [ "$ok" = 1 ]; then
        PASS=$((PASS + 1))
        printf '  [ OK ] %s\n' "$name"
    else
        FAIL=$((FAIL + 1))
        FAILED+=("$name")
        printf '  [FAIL] %s - %s\n' "$name" "$reason"
    fi
}

F="data/hopf01.dat"
FN="data/hopf01noisy.dat"

# --- usage ---------------------------------------------------------------
check "usage-no-args"          1 "" "" "USAGE" "" "" 0 -1
check "usage-too-many-args"    1 "" "$F extra.dat more.dat" "USAGE" "" "" 0 -1

# --- startup errors --------------------------------------------------------
check "missing-file"           1 "" "data/nonexistent.dat" "File not found" "" "" 0 -1
check "mode2-bad-noisy-file"   1 "" "$F data/nonexistent.dat" "File not found" "" "" 0 -1

# --- mode 1 happy paths ----------------------------------------------------
check "mode1-happy"            0 "H\n1\n10\nE\n" "$F" "Recall quality" "" "10% noisy pixels" 1 -1
check "mode1-default-rule"     0 "\n1\n10\nE\n" "$F" "Recall quality" "" "" 0 -1
check "mode1-storkey"          0 "S\n1\n10\nE\n" "$F" "Storkey" "" "" 0 -1
check "mode1-pseudo-inverse"   0 "P\n1\n10\nE\n" "$F" "pseudo-inverse" "" "" 0 -1
check "banner-version"         0 "\n1\n10\nE\n" "$F" "hopfieldann v[0-9]" "" "Number of neurons" 1 -1

# --- noise-level validation ------------------------------------------------
check "noise-too-high"         1 "P\n1\n150\nE\n" "$F" "out of range (0..100)" "" "" 0 -1
check "noise-negative"         1 "P\n1\n-5\nE\n" "$F" "out of range" "" "" 0 -1
check "noise-non-numeric"      1 "P\n1\nabc\nE\n" "$F" "invalid input" "" "" 0 -1
check "noise-zero"             0 "P\n1\n0\nE\n" "$F" "0% noisy pixels" "anti-correlated" "" 0 -1
check "noise-50"               0 "P\n1\n50\nE\n" "$F" "50% noisy pixels" "anti-correlated" "" 0 -1
check "noise-75-note"          0 "P\n1\n75\nE\n" "$F" "75% noisy pixels" "" "anti-correlated" 1 -1
check "noise-100-note"         0 "P\n1\n100\nE\n" "$F" "100% noisy pixels" "" "anti-correlated" 1 -1

# --- pattern-index validation ------------------------------------------------
check "index-too-low"          1 "P\n0\nE\n" "$F" "index 0 out of range" "" "" 0 -1
check "index-too-high"         1 "P\n99\nE\n" "$F" "index 99 out of range" "" "" 0 -1
check "index-non-numeric"      1 "P\nabc\nE\n" "$F" "invalid input" "" "" 0 -1

# --- repeat semantics: R and Enter re-run without re-prompting ---------------
check "repeat-R"               0 "P\n1\n15\nR\nE\n" "$F" "Recall quality" "" "15% noisy pixels" 2 -1
check "repeat-R-prompts-once"  0 "P\n1\n15\nR\nE\n" "$F" "" "" "Choose pattern to disturb" 1 1
check "repeat-enter"           0 "P\n1\n15\n\nE\n" "$F" "Recall quality" "" "15% noisy pixels" 2 -1
check "repeat-enter-prompts-once" 0 "P\n1\n15\n\nE\n" "$F" "" "" "Choose pattern to disturb" 1 1

# --- L reload ---------------------------------------------------------------
check "reload-L"               0 "P\n1\n15\nL\ndata/hopf03.dat\n1\n15\nE\n" "$F" "= 256" "" "Recall quality" 1 -1
check "reload-nonexistent"     1 "P\n1\n15\nL\ndata/nope.dat\nE\n" "$F" "File not found" "" "" 0 -1

# --- mode 2 ----------------------------------------------------------------
check "mode2-happy"            0 "\n1\nE\n" "$F $FN" "Recall quality" "" "2D image" 1 -1
check "mode2-noisy-count"      0 "\n1\nE\n" "$F $FN" "Number of noisy patterns" "" "" 0 -1
check "mode2-index-out-of-range" 1 "\n9\nE\n" "$F $FN" "index 9 out of range" "" "" 0 -1

# ---------------------------------------------------------------------------
rm -f "$OUT"
if [ "$FAIL" -eq 0 ]; then
    printf 'CLI contract: %d cases, all passed.\n' "$PASS"
    exit 0
fi

printf 'CLI contract: %d passed, %d FAILED: %s\n' "$PASS" "$FAIL" "${FAILED[*]}"
exit 1
