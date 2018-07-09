
Hopfield Artificial Neural Network
==================================

Console application implemented in C.

A Hopfield network is a recurrent artificial neural network (ANN) and was
invented by John Hopfield in 1982. A Hopfield network is a one layered network.
Every neuron is connected to every other neuron except with itself.

Every connection is represented by a weight factor. These weight factors are
determined by the Hebb's learning rule (1949). It is often summarized as
"Neurons that fire together, wire together. Neurons that fire out of sync, fail to link".

Associative memory maximum capacity: 0.14 * number of neurons.

Input format
------------

Four black-and-white images 10 x 12 ( . black  * white pixel) in plain ASCII text:

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

Building
--------

Use Cmake to build the application in the build directory:

    mkdir build
    cd build
    cmake ..
    make

Executing
---------

If you are in the build directory:

    ./hopfieldann ../data/hopf02.dat

Output
------

    Hopfield's ANN Simulation: Associative Memory HopfieldNN v1.3.4

    - Patterns file name: ../data/hopf02.dat   loading .... ready
    - Number of neurons: 10 * 12 = 120, number of patterns: 4
    - Learning patterns by hebbian learning rule .... ready
    - Learning result: connection matrix, size 120 x 120

    - Choose pattern to disturb by noise, index (1..4): 3

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

    - Noise [%]: 30

    - Pattern as 2D image and noisy pixels:

    *..**.*.**.*     ##  # #   #
    .**.....*...            #
    ..**...**...           ##
    ...**..*....           #
    ...*.**.**..        ### ##
    ..**..*.**.*          # ## #
    .***..******       #  ######
    **.***.**...      #   #  #
    .***....*...     ###    #
    ..*..*...**.      #  #   ##

    ---- Energy: -10.083333

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

    ---- Energy: -77.016667

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

    ---- Energy: -77.016667

    - E(xit), L(oad new patterns data file), N(ext simulation) .....
