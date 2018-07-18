# Hopfield Artificial Neural Network

[![Build Status](https://travis-ci.org/josokw/HopfieldANN.svg?branch=master)](https://travis-ci.org/josokw/HopfieldANN)

A Hopfield network is a recurrent artificial neural network (ANN) and was
invented by John Hopfield in 1982. A Hopfield network is a one layered
network.
Every neuron is connected to every other neuron except with itself.

Every connection is represented by a weight factor. These weight factors
are determined by the Hebb's learning rule (1949). It is often summarized
as "Neurons that fire together, wire together. Neurons that fire out of
sync, fail to link".

Maximum associative memory capacity: Pmax = 0.14 * number of neurons.
Above this maximum the network is no longer capable to work as an associative memory. 
If 120 neurons are available the Pmax equals 16 patterns. 

## Input format

Example plain ASCII input file:

- binary pattern 4 black-and-white images 10 x 12
- . black pixel
- \* white pixel

The first line must be available and shows the size of each pattern and
the number of patterns.

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

After reading each input pattern the weigth factors are updated by the
Hebbian learning rule in the same symmetric connection matrix W.
Because the neurons are not connected to itself the matrix W has a zero
valued diagonal.

## Building

The application does not use dynamic memory allocation.

Use *CMake* and *make* to build the application:

    mkdir build
    cd build
    cmake ..
    make

The excutable can be found in the *bin* directory.

## Executing

If the *build* directory is the current directory:

    ../bin/hopfieldann ../data/hopf02.dat

## Output

The application shows the recovering from a distorted (kwonw) input pattern to the
trained state that is most similar to that input.
Hopfield networks have a scalar value associated with each state of the
network, referred to as the "energy" of the network.
Repeated updating of the network by using the output as an input, will
eventually converge to a state which is a local minimum in the energy
function.

    Hopfield's ANN associative memory: hopfieldann v1.4.0

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

    ******.*.*..          # #
    .****.......       ##
    *****...*..*    ##  #   #  #
    *..**.*..*..    #     #  #
    .***.*..*.*.     ## ##  # #
    .****...*.**     #  #   # ##
    .**.....*..*            #  #
    .*******..*.    #       ###
    ..****.*.*.*      #### # # #
    ............

        Energy =   -8.6167

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

        Energy =  -77.0167

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

        Energy =  -77.0167


    - E(xit), L(oad new patterns data file), N(ext simulation) .....

Not all associated output patterns will be in every detail the same as the
learned input patterns.
Sometimes the associated output can be something we hasn't taught it.
"Hallucinations" is one of the main problems.
A Hopfield network can not tell you if the association is an
"hallucination".
