# AGENTS.md

## Project Overview

A text-console application written in **C17** implementing a Hopfield
recurrent neural network (1982). It trains on ASCII pattern files
(binary `*` / `.` images) and recovers noisy versions of stored images via
asynchronous (random sequential) recall dynamics. Recall quality is
reported as overlap ([-1,1]) and Hamming distance ([0,N]).

## Build

CMake >= 3.15, C17 (C++17 needed only for the Google Test binary).

```bash
mkdir build && cd build
cmake ..
make -j
```

- Executable: `bin/hopfieldann` · static lib: `build/libhopfield_lib.a`
- Linked: `m` (math)
- Warning flags (non-MSVC): `-Wall -Wextra -Wconversion -Werror=implicit-function-declaration`
- Install targets: `hopfieldann` executable and the `data/` directory.
- If adding a source file, extend `LIB_SOURCE_FILES` in `CMakeLists.txt`.

## Run

```bash
../bin/hopfieldann ../data/hopf01.dat [../data/hopf01noisy.dat]
```

- Arg 1: training-patterns file (required). Arg 2: noisy file → mode 2.
- Menu: `E(xit)`, `L(oad new file)` mode 1 only, `N(ext simulation)`,
  `R(un again)`, Enter = repeat last simulation.

## Tests

```bash
cd build && cmake .. && make -j && ctest --output-on-failure
```

Two CTest tests:

1. `HopfieldTest` — Google Test at `test/testHopfieldCalc.cpp`, **57 `TEST_F`
   cases** across two suites:
   - `HopfieldCalcTest` — util helpers, learning rules (all five, incl.
     pseudo-inverse rejection of linearly dependent patterns), symmetry/diagonal
     checks, energy, async/sync recall, noise, overlap/Hamming, storage capacity,
     context resize.
   - `HopfieldIOTest` — file parsing: success, missing file, dimension mismatch,
     overlong/short rows, illegal chars, truncated rows, CRLF, blank-line skipping,
     header junk/partial headers, growth beyond old fixed limits.
   - Fixture creates/destroys a `HopfieldContext` in `SetUp`/`TearDown`; helper
     `allocate(patternSize, nPatterns, nNoisy=0)` wraps `hopfield_context_resize`.
     Run directly: `./build/hopfield_test`.
2. `CliContract` — `testScripts/cli_contract_test.sh <path-to-binary>`
   (Unix only): pipes stdin into the binary, asserts exit codes, key output lines,
   and occurrences (usage, error paths, noise/index validation, R/Enter repeat,
   L reload, capacity warning, mode 2). Run from the project root.

## Input data format

- First line: header `<rows> <columns> <npatterns>` — exactly 3 integers; any
  trailing non-blank content on that line is rejected (`HOPFIELD_ERR_INVALID_FORMAT`).
- Each pattern is `rows` lines of exactly `columns` characters (blank lines between
  patterns allowed and skipped). Encoding: `*` → `+1`, `.` → `-1`; any other
  character in a row is an error.
- CRLF/CR line endings are accepted; a trailing blank line after the last pattern
  is tolerated. `rows`/`columns` must be positive; `npatterns` must be positive
  (`HOPFIELD_ERR_SIZE_EXCEEDED`).
- The noisy file (mode 2) must have the same `rows`/`columns` as the training file.
- All pattern/W storage is sized dynamically from the header — no fixed limits.

## Error model

`HopfieldError` enum in `src/HopfieldContext.h`:

| Value | Meaning |
|-------|---------|
| `HOPFIELD_OK` | success |
| `HOPFIELD_ERR_FILE_NOT_FOUND` | `fopen` failed |
| `HOPFIELD_ERR_INVALID_FORMAT` | header/row parse failure, wrong noisy-file dims |
| `HOPFIELD_ERR_INDEX_OUT_OF_RANGE` | bad pattern index |
| `HOPFIELD_ERR_SIZE_EXCEEDED` | non-positive `npatterns` (or similar) |
| `HOPFIELD_ERR_OUT_OF_MEMORY` | allocation failure |

The CLI exits `0` on success and `1` on usage errors or any failure. Every
heap allocation must be NULL-checked and mapped to `HOPFIELD_ERR_OUT_OF_MEMORY`
(or `false` for the `bool` learning/recall functions).

## CLI behavior

- Startup `srand(time(NULL))` is called exactly once — recall uses randomized
  (asynchronous) updates, so output is non-deterministic.
- Learning-rule prompt `(H)ebbian, (S)torkey, (P)seudo-inverse, (D)aydreaming or
  (M)odern? [H]` — blank/EOF selects Hebbian.
- **Mode 1** (`argc == 2`): prompts for pattern index `1..nPatterns` then noise
  level `0..100`; corrupts the chosen pattern in memory via `addNoiseToPattern`.
- **Mode 2** (`argc == 3`): prompts for a noisy-pattern index `1..nNoisyPatterns`;
  no noise prompt (noise is baked into the input file).
- Noise above `MAX_INFORMATIVE_NOISE_PERCENT` (50) prints an anti-correlation note
  but is accepted; values outside `0..100` are rejected with
  `"noise level %d out of range (0..100)"`.
- Invalid index input prints `"index %d out of range"`; non-numeric input prints
  `"invalid input"`. `R`/Enter re-run the last simulation without re-prompting.
- `L` reload (mode 1) reads a file name (max `MAXFILENAME_SIZE - 1` = 99 chars;
  longer names trigger `"file name too long"`), then re-learns with the current rule.

## Code Layout

- `src/main.c` — entry point: create context, `run_cli`, destroy.
- `src/cli.c` / `cli.h` — interactive menu + simulation loop.
- `src/HopfieldContext.{h,c}` — `HopfieldContext` struct, `HopfieldError`
  enum, `allocMatrix`/`freeMatrix`, context create/destroy/resize.
- `src/HopfieldCalc.{h,c}` — math: five learning rules, recall, energy,
  `convergePattern()` convergence engine, overlap/Hamming.
- `src/HopfieldIO.{h,c}` — pattern-file parsing, grid display (`show*`).
- `src/HopfieldUtil.{h,c}` — `equals`, `sign`, `randomInt`, `copyPattern`.
- `src/HopfieldConfig.h` — tunable constants (iteration/noise caps,
  daydreaming + modern-Hopfield params).
- `src/AppInfo.h` — app name/version macros (bump here on change).
- `data/` — sample patterns · `test/` — gtest · `testScripts/` — CLI tests.

## Conventions

- **Naming:** public functions camelCase (`learnHebbian`, `calcEnergy`,
  `readFile`, `hopfield_context_create`); error enum `HOPFIELD_OK`,
  `HOPFIELD_ERR_*`; cli.c static helpers snake_case (`run_cli` is the
  public exception).
- **C/C++ interop:** public headers wrapped in `extern "C"` for the test.
- **Memory:** all pattern/W storage heap-allocated and sized from the file
  header — no fixed limits. NULL-check every allocation; return
  `HOPFIELD_ERR_OUT_OF_MEMORY` on failure.
- **Floating point:** compare with `equals()` (epsilon), never `==`.
- **Formatting:** `.clang-format` → 3-space indent, 75-column limit, Allman
  braces on functions, `else` on its own line.
- **Learning rules** (selected at startup, re-run on reload): Hebbian (default,
  outer product normalized by N, zero diagonal); Storkey (local-field crosstalk
  correction); pseudo-inverse (builds W from `(1/N)·Ξ·G⁻¹·Ξᵀ`, keeps a **non-zero
  diagonal**); Daydreaming (Hebbian baseline + `DAYDREAMING_EPOCHS` epochs of N
  reinforce/unlearn steps, renormalized by spectral norm each epoch; slowest rule);
  Modern Hopfield (store patterns as memory — recall is `Ξ·softmax(β·Ξᵀx)`, W
  keeps the Hebbian values for display).
- Recall dispatches on `ctx->modernHopfield`: when true only
  `convergeModernPattern()` (softmax update) may be used; otherwise W-based async
  recall.

## Recall & convergence

- `convergePattern()` — the classical convergence engine. Random-sequential
  (asynchronous) updates; loops until a **full sweep produces no flips** (a true
  fixed point) or `MAX_ITERATIONS` (1000) is exhausted. Reports energy via a
  `ConvergenceCallback(iteration, energy, pattern, user_data)` — pass `NULL` for
  headless use (tests, Daydreaming training).
- Modern path — iterate `x ← Ξ·softmax(β·Ξᵀx)` until
  `max|xᵢ − prevᵢ| < MODERN_CONVERGENCE_EPSILON` (1e-6), then threshold the
  output back to ±1 so display/metrics stay valid.
- `addNoiseToPattern(ctx, pat, chance)` — randomly flips `chance`% of the chosen
  pattern's bits; clamps `chance` to `[0,100]`.
- Energy: `calcEnergy` (quadratic, W-based) or `calcModernEnergy`
  (`E(x) = −lse(β, Ξᵀx) + ½‖x‖²`). Metrics after recall:
  `overlap = (1/N)Σ xᵢ·yᵢ` ∈ [-1,1], `hamming ∈ [0,N]`, related by
  `overlap = 1 − 2·hamming/N`.
- `storageCapacity(N) = max(1, floor(0.138·N))`; the CLI warns
  `"associative storage capacity N exceeded"` during `prepare_network` when the
  pattern count exceeds it.

## Tuning constants (`src/HopfieldConfig.h`)

- `MAX_ITERATIONS` (1000), `MAX_NOISE_PERCENT` (100),
  `MAX_INFORMATIVE_NOISE_PERCENT` (50), `STORAGE_CAPACITY_FACTOR` (0.138).
- Daydreaming: `DAYDREAMING_EPOCHS` (10), `DAYDREAMING_TAU_FACTOR` (1.0),
  `DAYDREAMING_NORM_ITERATIONS` (50).
- Modern: `MODERN_BETA` (1.0), `MODERN_CONVERGENCE_EPSILON` (1e-6).
- Version number lives in `src/AppInfo.h` — bump `MAJOR/MINOR/REVISION` on changes.

## Invariants — do not break

- Weight matrix `W` must stay symmetric and (except as noted) zero-diagonal;
  `learnHebbian` asserts both post-training.
- Pseudo-inverse deliberately keeps a **non-zero diagonal**; Modern Hopfield
  recalls from stored patterns directly (W keeps Hebbian values). Recall may
  only use W when `ctx->modernHopfield` is false.
- `convergePattern()` must end only after a full asynchronous sweep with no
  flips (i.e., a true fixed point). Pseudo-inverse rejects linearly dependent
  patterns ("Failed to learn patterns").
- Do not add fixed-size arrays based on "enough for current data" — storage is
  meant to scale from the file header.
- Never commit secrets; check nothing new goes into `bin/`, `build/`,
  `graphify-out/`, or the `session-ses_*.md` session logs (all gitignored) unless
  a tracked copy is intended.