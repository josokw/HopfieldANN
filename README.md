# Hopfield Artificial Neural Network

The Hopfield ANN application is a text console application implemented in C11 showing the recovery
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

More detailed information reference:
[Hopfield Wikipedia](https://en.wikipedia.org/wiki/Hopfield_network).

## Input example for learning (training) 4 patterns

Example input format plain ASCII input file for training the network: **supervised learning**.

- 4 binary patterns (black-and-white images) 10 x 12
- . black pixel
- \* white pixel

The first ine must specify and shows the size of each pattern and the number of patterns.

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

```text
Hopfield's ANN associative memory: hopfieldann v1.5.0

- Input patterns file name: data/hopf01.dat, loading .... ready
- Number of neurons: 10 * 10 = 100, number of patterns = 7
- Learning patterns by Hebbian learning rule, training starts .... ready
- Learning result: 1 connection matrix, size 100 x 100

- Choose pattern to disturb by noise, index (1..7): 3

..........
**********
.***......
..***.....
...***....
...***....
...***....
..***.....
.***......
**********

- Noise level [%]: 25


- Pattern as 2D image and noisy pixels:

*....*...*    #    #   #
**.***.***      #   #
..**...*..     #     #
..**......        #
.....*..*.       ##   #
..***.....      #  #
..*.*..**.      ## # ##
..*.*..*..       #   #
.**......*       #     #
**.******.      #      #

    Energy =  -15.2600

..........
*.*.******     # #
.***......
....*.....      ##
...***....
...***....
...***....
..**......        #
.***......
**********

    Energy =  -46.8400

..........
**********
.***......
..***.....
...***....
...***....
...***....
..***.....
.***......
**********

    Energy =  -51.9000

..........
**********
.***......
..***.....
...***....
...***....
...***....
..***.....
.***......
**********

    Energy =  -51.9000


- E(xit), L(oad new patterns data file), N(ext simulation) ....
```

Not all associated output patterns will be in every detail the same as the learned input patterns.
Sometimes the associated output can be something it was never taught. "Hallucinations" is one of the
main problems. A Hopfield network cannot tell you if the association is a "hallucination".

## Building: using C11, CMake and make

The application does not use dynamic memory allocation. The maximum number of neurons and the
maximum number of input learning patterns can be configured in the C file _HopfieldConfig.h_.

Go to the **HopfieldANN** directory. Use _CMake_ and _make_ to build the application:

    git clone https://github.com/josokw/HopfieldANN.git
    cd HopfieldANN
    mkdir build && cd build
    cmake ..
    make -j

The executable can be found in the _bin_ directory.

## Executing: using an input file containing patterns to learn

Some input file examples can be found in the _data_ directory.

Starting the program without parameters will show the usage help:

    USAGE: hopfieldann <patterns filename>

    USAGE: hopfieldann <patterns filename> <noisy patterns filename>

If the _build_ directory is the current directory:

    ../bin/hopfieldann ../data/hopf01.dat

    ../bin/hopfieldann ../data/hopf01.dat ../data/hopf01noisy.dat

## Test scripts

Using file redirection for generating test data:

    ../bin/hopfieldann ../data/hopf01.dat < ../testScripts/input_test_1.txt > ../testScripts/result_test_1.txt

## License

See the [License](./License) file.

## Updates guided by OpenCode (May 2026)

As of May 2026, the adoption of AI technologies had accelerated significantly. This small C project
serves as a practical case study for applying AI-guided improvements.

Used **big-pickle (opencode/big-pickle)**, a large language model fine-tuned specifically for software
engineering tasks within the opencode ecosystem.

Decoder-only transformer with:

- 120B parameters
- 65+ layers
- 8K+ context window (varies by deployment)
- Sparse mixture-of-experts (MoE) feed-forward layers for compute efficiency
- Grouped-query attention (GQA) with 8 key/value heads
- Rotary position embeddings (RoPE)

In June 2026 **Nemotron 3 Ultra Free OpenCode Zen** and **Nemotron 3 Super Nvidia** were also used.
