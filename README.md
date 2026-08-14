# Hopfield Artificial Neural Network

The Hopfield ANN application is a text console application implemented in C17 showing the recovery
of noisy (damaged) learned images.

[![CodeFactor](https://www.codefactor.io/repository/github/josokw/hopfieldann/badge)](https://www.codefactor.io/repository/github/josokw/hopfieldann)

## Hopfield recurrent artificial neural network

A Hopfield network is a recurrent artificial neural network (ANN) and was invented by John Hopfield
in 1982. A Hopfield network is a single-layer network. Every neuron is connected to every other
neuron except with itself. The neurons have a **binary output** taking the values –1 and 1.

Every connection is represented by a **weight factor**. These weight factors are determined by the
Hebb's learning rule (1949). It is often summarized as "Neurons that fire together, wire together.
Neurons that fire out of sync, fail to link".

These connections are implemented in a symmetric and zero valued diagonal **matrix W**:

    W[i][j] == W[j][i]

    W[i][i] == 0

Maximum associative memory capacity:

    Pmax = 0.14 * number of neurons

Above this maximum the network is no longer capable of working as an associative memory. If 120
neurons are available the Pmax equals 16 patterns that can be stored and retrieved.

**Note**: Patterns should be chosen so that no pattern is a linear superposition (summation) of
others. When patterns are strongly correlated, crosstalk during recall increases and the network
is more likely to converge to spurious attractors instead of the correct stored pattern.

## Learning rules

At startup, the program prompts the user to choose a learning rule.

### Hebbian learning rule

The default learning rule. Weight updates follow Hebb's rule — neurons that fire together,
wire together. The weight matrix is updated for each pattern using the outer product
normalized by pattern size:

    W_ij = (1/N) Σ_μ x_i^μ · x_j^μ    (i ≠ j),   W_ii = 0

This rule is simple and biologically plausible, but crosstalk between correlated patterns
can reduce recall accuracy.

### Storkey learning rule

The Storkey learning rule (also known as the Storkey–Palmer rule) reduces crosstalk between
stored patterns by incorporating a local-field correction during training. For each pattern,
the weight update is computed using the pre-synaptic local field, which subtracts the
influence of existing weights before adding the new pattern:

    h_j = Σ_k W_ik · x_k^μ    (k ≠ i,j)
    ΔW_ij = (1/N) (x_i^μ · x_j^μ − x_i^μ · h_j − h_i · x_j^μ)

This results in a weight matrix with reduced interference, higher practical storage capacity,
and fewer spurious attractors compared to the Hebbian rule. The interactive prompt defaults
to Hebbian if only Enter is pressed.

More detailed information reference:
[Hopfield Wikipedia](https://en.wikipedia.org/wiki/Hopfield_network).

### Pseudo-inverse (Moore–Penrose) learning rule

The pseudo-inverse learning rule (Personnaz, Guyon & Toulouse, 1986) computes the weight
matrix directly from the Gram matrix of the stored patterns instead of accumulating weight
updates pattern by pattern:

    G[μ][ν] = (1/N) Σ_i ξ^μ_i · ξ^ν_i
    W = (1/N) Ξ · G⁻¹ · Ξᵀ

Every stored pattern is then an **exact eigenvector of W with eigenvalue 1**, so each learned
pattern is a true fixed point of the recall dynamics. This gives the network near-perfect
recall and a practical storage capacity approaching N − 1 (far above the ~0.14·N limit of the
Hebbian rule), provided the patterns are linearly independent.

Note that, unlike the Hebbian and Storkey rules, the pseudo-inverse rule produces a **non-zero
diagonal** in the weight matrix. The diagonal is intentionally kept: zeroing it would turn the
exact fixed points into only approximate ones. The recall and energy machinery does not require
a zero diagonal, only symmetry.

Caveat: the Gram matrix G must be invertible, so patterns that are linearly dependent (e.g.,
duplicate patterns or a pattern equal to the negative of another) are rejected at training time
with a "Failed to learn patterns" error. Strongly correlated patterns remain problematic as with
any learning rule.

The interactive prompt accepts `P` for this rule:

    Learning rule: (H)ebbian, (S)torkey, (P)seudo-inverse or (D)aydreaming? [H]:

### Daydreaming learning rule

The Daydreaming learning rule (Serricchio et al., "Daydreaming Hopfield Networks and their
surprising effectiveness on correlated data", *Neural Networks* 186 (2025) 107216) starts from
the Hebbian coupling matrix and then repeatedly applies an iterative update that, at every
step, reinforces a randomly chosen stored pattern while *unlearning* the spurious attractor
reached by the zero-temperature dynamics from a random initial configuration:

    J_ij ← J_ij + (1/(τ·N)) (ξ^μ_i·ξ^μ_j − σ_i·σ_j)

where μ is a pattern picked uniformly at random, σ is the fixed point reached from a random
state, and τ is an inverse learning rate. After each epoch of N steps the coupling matrix is
renormalized by its spectral norm ‖J‖₂ (computed by power iteration). The procedure needs no
fine-tuning — for large enough τ the result is τ-independent — and can be iterated indefinitely
without the destructive forgetting that plagues plain dreaming/unlearning rules.

Daydreaming produces larger basins of attraction and higher practical storage capacity than the
Hebbian rule, works well on correlated patterns, and is fully local (each weight update depends
only on neuron values). The diagonal stays exactly zero because ξ_i² − σ_i² = 0. The default
training uses 10 epochs with τ = N, tunable via `DAYDREAMING_EPOCHS` and `DAYDREAMING_TAU_FACTOR`
in `src/HopfieldConfig.h`.

The interactive prompt accepts `D` for this rule. Note that training performs N dynamics runs
per epoch, so it takes noticeably longer than the one-shot rules on larger grids.

> **Known performance issue**: The current implementation runs full asynchronous convergence
> (up to 1000 sweeps) at each of the `DAYDREAMING_EPOCHS × N` steps, making Daydreaming
> unusably slow for grids larger than ~50 neurons. For interactive use on the provided
> demo data (100–256 neurons), prefer **Hebbian, Storkey, Pseudo-inverse, or Modern**
> learning rules. A fix replacing full convergence with a single async sweep per step is
> planned.

### Modern Hopfield (softmax / exponential-energy) learning rule

The Modern Hopfield learning rule (Ramsauer et al., "Hopfield Networks is All You Need",
2020) stores the patterns **themselves** as the memory and retrieves with a single
softmax-attention read-out over continuous states:

    x ← Ξ · softmax(β · Ξᵀ x)          with  E(x) = −lse(β, Ξᵀx) + ½‖x‖²

where Ξ is the N×M matrix of stored patterns, β is the inverse softmax temperature
(default `MODERN_BETA = 1.0` in `src/HopfieldConfig.h`; larger β means sharper,
near-argmax retrieval), and `lse` is the log-sum-exp computed with a stable
max-subtraction so `exp(β·N)` never overflows.

This is the model behind the **2024 Nobel Prize in Physics** line of work: its update rule is
mathematically identical to the attention mechanism inside Transformers. Because the
energy is exponential rather than quadratic, the practical storage capacity grows far
beyond the classical ~0.14·N limit — in tests, 32 random patterns on 64 neurons (more than
5× the classical capacity) are all retrieved exactly. Clean inputs settle in a single
update; noisy inputs need one or two. The final output is thresholded back to ±1 so the
grid display and the overlap/Hamming metrics work unchanged.

Note that, as with the pseudo-inverse rule, the recall machinery does not use the symmetric
weight matrix `W`; `W` is still filled with the Hebbian coupling matrix so the connection
matrix invariants and output messages stay consistent, but the stored patterns are the
actual memory.

The interactive prompt accepts `M` for this rule:

    Learning rule: (H)ebbian, (S)torkey, (P)seudo-inverse, (D)aydreaming or (M)odern? [H]:

## Input example for learning (training) 4 patterns

Example input format plain ASCII input file for training the network: **supervised learning**.

- 4 binary patterns (black-and-white images) 10 x 12
- . black pixel
- \* white pixel

The first line must specify the size of each pattern and the number of patterns.

        10 12 4

        .*....*.....
        ************
        .*....*.....
        .*....*.....
        .*....*.....
        .*....*.....
        ************
        .*....*.....
        .*....*.....
        .*....*.....

        ......******
        .....***....
        ....***.....
        ...***......
        ..***.......
        .***........
        ***.........
        **..........
        ............
        ............

        **********..
        .**.........
        ..**........
        ...**.......
        ...**.......
        ..**........
        .**.........
        **********..
        ............
        ............

        ........***.
        .......***..
        ......***...
        .....***....
        ....***.....
        ...***......
        ..***.......
        .***........
        ***.........
        **..........

After reading each input pattern the weight factors are updated by the Hebbian learning rule in the
same symmetric connection matrix W. Because the neurons are not connected to itself the matrix W has
a zero valued diagonal.

## Output pattern recognition

The application shows the recovering from a distorted known input pattern to the trained state that
is most similar to that input. Hopfield networks have a scalar value associated with each state of
the network, referred to as the "energy" of the network. Repeated updating of the network by using
the output as an input, will eventually converge to a state which is a local minimum in the energy
function. The output shows # for changed pixels.

See [testScripts/result_test_1.txt](testScripts/result_test_1.txt).

Not all associated output patterns will be in every detail the same as the learned input patterns.
Sometimes the associated output can be something it was never taught. "Hallucinations" is one of the
main problems. A Hopfield network cannot tell you if the association is a "hallucination".

### Asynchronous updates

The network uses **asynchronous (random sequential) updates** instead of synchronous (parallel)
updates. In each sweep, one neuron is randomly selected, its local field is computed from the
current state of all other neurons, and it is updated immediately before the next neuron is chosen.

This reduces **spurious attractors** — stable states that were never learned. Synchronous updates
can cause neurons to overshoot, leading to oscillations between non-learned states. Asynchronous
updates eliminate oscillations entirely because each neuron uses the most recent values of its
inputs. The energy is guaranteed to decrease (or stay the same) after every single-neuron flip,
so the network always converges to a fixed point rather than cycling.

Note that some spurious attractors can still exist as fixed points (e.g., inverted patterns or
mixtures of stored patterns), but their number is significantly reduced compared to synchronous
updates.

### Recall quality: overlap and Hamming distance

After retrieval, the application reports **overlap** and **Hamming distance** between the
converged pattern and the original learned pattern:

- **Overlap** `∈ [-1, 1]`: the normalized dot product `(1/N) Σ xᵢ·yᵢ`. `+1` = identical,
  `0` = orthogonal, `-1` = inverted.
- **Hamming distance** `∈ [0, N]`: the number of mismatching bits. `0` = perfect recall.

The two metrics are related by `overlap = 1 − 2 × Hamming / N`. A small Hamming distance
(e.g., `1 / 100`) means the converged pattern is nearly identical to the stored one, while
a large distance indicates convergence to a spurious attractor or the wrong learned pattern.

## Building: using C17, CMake and make

**Prerequisites:** CMake >= 3.15, a C17-compatible compiler, and Google Test
(`libgtest-dev` on Debian/Ubuntu, `gtest` on macOS via Homebrew).

All storage (patterns, noisy patterns and the weight matrix) is allocated
dynamically at load time based on the input file header, so there is no fixed
maximum for the number of neurons or the number of learning patterns.

Go to the **HopfieldANN** directory. Use *CMake* and *make* to build the application:

    git clone https://github.com/josokw/HopfieldANN.git
    cd HopfieldANN
    mkdir build && cd build
    cmake ..
    make -j

The executable can be found in the *bin* directory.

## Executing: using an input file containing patterns to learn

Input file examples can be found in the *data* directory:

- `hopf01.dat` — 10×10, 7 patterns (letters and shapes)
- `hopf01noisy.dat` — 10×10, 4 noisy variants of hopf01
- `hopf02.dat` — 10×12, 4 patterns (letters and shapes)
- `hopf03.dat` — 16×16, 6 patterns (geometric shapes)
- `hopf04.dat` — 10×10, 5 patterns (diverse symbols)

### Interactive mode

Starting the program without parameters will show the usage help:

    USAGE: hopfieldann <patterns filename>

    USAGE: hopfieldann <patterns filename> <noisy patterns filename>

If the *build* directory is the current directory:

    ../bin/hopfieldann ../data/hopf01.dat

    ../bin/hopfieldann ../data/hopf01.dat ../data/hopf01noisy.dat

### Batch mode (non-interactive)

Run single or multiple simulations without entering the interactive menu:

    # Single pattern, Hebbian, 10% noise
    hopfieldann patterns.dat --rule hebbian --pattern 1 --noise 10

    # Multiple patterns, Storkey, reproducible seed
    hopfieldann patterns.dat --rule storkey --pattern 1,3,5 --noise 20 --seed 42

    # Mode 2 with pre-made noisy file
    hopfieldann patterns.dat noisy.dat --pattern 1,2

    # Save/load weight matrix
    hopfieldann patterns.dat --rule modern --pattern 1 --noise 10 --save-weights weights.bin
    hopfieldann patterns.dat --load-weights weights.bin --pattern 1 --noise 10

    # Quiet mode (exit code only)
    hopfieldann patterns.dat --rule hebbian --pattern 1 --noise 10 --quiet

    # Verbose (show energy per iteration)
    hopfieldann patterns.dat --rule hebbian --pattern 1 --noise 10 --verbose

    # Save recall output to file
    hopfieldann patterns.dat --rule hebbian --pattern 1 --noise 10 --output recall.dat

## Interactive menu

After training, the program presents a menu loop. The available options depend on the
invocation mode:

**Mode 1** (`hopfieldann <patterns>`):

    E(xit), L(oad new patterns data file), N(ext simulation), R(un again)

**Mode 2** (`hopfieldann <patterns> <noisy>`):

    E(xit), N(ext simulation), R(un again)

| Key    | Action                           |
|--------|----------------------------------|
| `E`/`e`| Exit the program                 |
| `L`/`l`| Load a new pattern file (mode 1) |
| `N`/`n`| Start a new simulation           |
| `R`/`r`| Re-run the last simulation       |
| Enter  | Alias for `R`                    |

In Mode 1, `N` prompts for a pattern index and noise percentage, corrupts the chosen
pattern on the fly, and runs recall. In Mode 2, `N` prompts for one of the pre-made
noisy patterns from the second input file.

Noise is expressed as a percentage in the range 0–100. Up to 50% the noisy input is still
correlated with the stored pattern and recall into its basin is expected. Above 50% the
input is anti-correlated (the network converges to the inverted pattern, since `W(-x) = -Wx`);
an informational note is printed and the value is accepted.

The `R` and Enter options repeat the previous simulation with the exact same parameters
without re-prompting.

### Simulation output

Each iteration displays the current pattern state, a difference column (showing `#` for
neurons that differ from the target), and the current energy:

    <pattern grid>    <difference grid>
        Energy = -30.9200

After convergence the program reports recall quality:

    Recall quality: overlap =  1.0000, Hamming distance = 0 / 100

A warning is shown if the number of stored patterns exceeds the theoretical capacity:

    Warning: associative storage capacity 13 exceeded

## Batch mode reference

### Command-line options

| Option | Description |
|--------|-------------|
| `-r, --rule RULE` | Learning rule: `hebbian`, `storkey`, `pseudo-inverse`, `daydreaming`, `modern` (default: interactive prompt) |
| `-p, --pattern LIST` | Pattern indices 1..N, comma-separated (e.g., `1,3,5`). In mode 2, uses noisy pattern indices. |
| `-n, --noise PERCENT` | Noise level 0–100 (mode 1 only) |
| `-s, --seed VALUE` | Random seed for reproducible runs |
| `-q, --quiet` | Suppress non-error output; only exit code indicates result |
| `-v, --verbose` | Show energy per iteration during convergence |
| `-o, --output FILE` | Save recall pattern(s) to FILE (appends for multiple patterns) |
| `-w, --save-weights FILE` | Save weight matrix after learning (binary `.bin` format) |
| `-l, --load-weights FILE` | Load weight matrix, skip learning |
| `-h, --help` | Show help |

### Exit codes (batch mode)

| Code | Meaning |
|------|---------|
| `0` | All patterns converged |
| `1` | Usage error, file error, or learning failed |
| `2` | At least one pattern did not converge |
| `3` | Invalid pattern index or noise value |

### Config file

Optional `.hopfieldrc` in the current directory or `$HOME`. Format:

```ini
rule = storkey
seed = 12345
noise = 15
verbose = true
save_weights = weights.bin
load_weights = weights.bin
output = recall.dat
```

CLI options override config file values.

## Test scripts

Using file redirection for generating test data:

    ../bin/hopfieldann ../data/hopf01.dat < ../testScripts/input_test_1.txt > ../testScripts/result_test_1.txt

## Running tests

The project includes a Google Test suite. From the build directory:

    cmake ..
    make -j
    ctest --output-on-failure

## License

See the [License](./License) file.

## Updates guided by opencode (2026)

Code quality improvements were performed using AI-assisted reviews with
**big-pickle (opencode/big-pickle)**.

Features added during this reviews:

- **Storkey learning rule** — alternative learning rule with local-field correction
  for reduced crosstalk between stored patterns
- **Pseudo-inverse (Moore–Penrose) learning rule** — weight matrix built from the inverse
  Gram matrix, making every stored pattern an exact fixed point (capacity up to N − 1,
  provided patterns are linearly independent)
- **Daydreaming learning rule** — iterative reinforcement of stored patterns plus
  unlearning of spurious attractors, with larger basins of attraction and higher
  capacity than Hebbian, including on correlated data
- **Modern Hopfield (softmax / exp-energy) learning rule** — pattern-based retrieval
  via the attention-equivalent update `x ← ��·softmax(β·�����x)`, with exponential
  storage capacity beyond the classical ~0.14·N limit
- **`convergePattern()`** — callback-based convergence engine that separates
  iteration logic from display, enabling reuse and testing
- **Overlap and Hamming distance** — quantitative recall quality metrics reported
  after each simulation
- **`R`(un again) and Enter(repeat)** — menu options to re-run the last simulation
  without re-entering parameters
- **Google Test suite** — 57 unit tests across two test suites for algorithmic
  correctness and I/O error handling
- **VS Code launch/tasks configuration** — integrated single-key build and debug
  support
- **Batch mode (v1.16.0)** — non-interactive CLI with `--rule`, `--pattern` (comma-separated),
  `--noise`, `--seed`, `--quiet`, `--verbose`, `--output`, `--save-weights`, `--load-weights`
- **Weight matrix I/O (v1.16.0)** — binary save/load of trained weights for reuse
- **Config file (v1.16.0)** — `.hopfieldrc` for persistent defaults
- **Phase 1 bug fixes (v1.15.2)** — atomic buffer allocation, noisyPatterns size handling,
  configurable pseudo-inverse threshold

Note: the Google Test suite has grown since; the convergence engine now terminates only when
a full asynchronous sweep produces no neuron flips, guaranteeing convergence to a true fixed
point. Since the 1.12.1 refactor all pattern and weight storage is dynamically sized from the
input file header, removing the previous hard-coded limits.
