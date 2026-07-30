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
to Hebbian if only Enter is pressed:

    Learning rule: (H)ebbian or (S)torkey? [H]:

More detailed information reference:
[Hopfield Wikipedia](https://en.wikipedia.org/wiki/Hopfield_network).

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

The maximum number of neurons and the maximum number of input learning patterns
can be configured in the C file _HopfieldConfig.h_.

Go to the **HopfieldANN** directory. Use _CMake_ and _make_ to build the application:

    git clone https://github.com/josokw/HopfieldANN.git
    cd HopfieldANN
    mkdir build && cd build
    cmake ..
    make -j

The executable can be found in the _bin_ directory.

## Executing: using an input file containing patterns to learn

Input file examples can be found in the _data_ directory:

- `hopf01.dat` — 10×10, 7 patterns (letters and shapes)
- `hopf01noisy.dat` — 10×10, 4 noisy variants of hopf01
- `hopf02.dat` — 10×12, 4 patterns (letters and shapes)
- `hopf03.dat` — 16×16, 6 patterns (geometric shapes)
- `hopf04.dat` — 10×10, 5 patterns (diverse symbols)

Starting the program without parameters will show the usage help:

    USAGE: hopfieldann <patterns filename>

    USAGE: hopfieldann <patterns filename> <noisy patterns filename>

If the _build_ directory is the current directory:

    ../bin/hopfieldann ../data/hopf01.dat

    ../bin/hopfieldann ../data/hopf01.dat ../data/hopf01noisy.dat

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

Code quality improvements were performed using AI-assisted review with
**big-pickle (opencode/big-pickle)**.

Features added during this review:

- **Storkey learning rule** — alternative learning rule with local-field correction
  for reduced crosstalk between stored patterns
- **`convergePattern()`** — callback-based convergence engine that separates
  iteration logic from display, enabling reuse and testing
- **Overlap and Hamming distance** — quantitative recall quality metrics reported
  after each simulation
- **`R`(un again) and Enter(repeat)** — menu options to re-run the last simulation
  without re-entering parameters
- **Google Test suite** — 17 unit tests across two test suites for algorithmic
  correctness and I/O error handling
- **VS Code launch/tasks configuration** — integrated single-key build and debug
  support
