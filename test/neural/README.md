# Neural Network Implementation and Benchmarking

This directory contains implementations of neural network algorithms and benchmarking tools to compare the precision of standard floating-point and HUB-format floating-point calculations in neural network operations.

## Overview

The neural network implementation provides a templated approach that works with different numeric types, including:
- Standard `float`
- Standard `double` 
- Custom `hub_float` type

The code is designed to compare numerical accuracy, training convergence, and inference performance between these types, with `double` precision used as the reference implementation.

## Files

- `neural.h`, `neural_impl.hpp`: Templated class and function declarations for neural networks
- `neural.cpp`: Implementation of neural network algorithms
- `mnist_loader.h`: Reader for the MNIST IDX files
- `half.hpp`: Half-precision (binary16) type used as a comparison format
- `main.cpp`: Benchmark program that compares `double`, `half` and `hub_float`

The MNIST data files are not included in the repository; see Usage.

## Neural Network Implementation

The implementation includes:
1. Feed-forward neural network with configurable layers
2. Backpropagation for training
3. Various activation functions (sigmoid, ReLU, tanh)
4. Common loss functions (MSE, cross-entropy)

## Attribution

This neural network implementation is based on code from the GitHub repository [NeuralNetworkInAllLangs](https://github.com/dlidstrom/NeuralNetworkInAllLangs), which is available under the MIT License. We have adapted the code for our specific benchmarking and other needs to compare floating-point precision across different numeric types. 

## Benchmark

The benchmark program:
- Trains a 784–128–10 network in `double` on 10 000 MNIST training images (5 epochs, batch 100, learning rate 0.1, fixed shuffle seed)
- Converts the trained weights to `half` and to `hub_float` and evaluates all three types on 1000 test images
- Reports the test-set accuracy of each type, and the RMSE of the `half` and `hub_float` outputs against the `double` network, both before and after the output sigmoid

## Usage

### Running the Benchmark

Build from the repository root with `make`, then run the binary from a directory containing the MNIST files:

```bash
make
cd /path/to/mnist
/path/to/HUBsim/bin/neural
```

The program reads `train-images-idx3-ubyte`, `train-labels-idx1-ubyte`, `t10k-images-idx3-ubyte` and `t10k-labels-idx1-ubyte` (uncompressed) from the current directory.
They are not distributed with the repository.

### Output

Console output only: per-epoch training loss and accuracy, then the accuracy of each type and the output RMSE of `half` and `hub_float` against `double`.

## Results


`half` is a 16-bit format, so it is compared against `hub_float` built at the same width (`make clean && make EXP_BITS=5 MANT_BITS=10`):

```
Double precision accuracy: 86.3%
Half precision accuracy: 86.2%
hub_float precision accuracy: 86.2%
Raw output RMSE (half-Double): 1.1211e-02
Raw output RMSE (hub_float-Double): 1.1179e-02
Sigmoid output RMSE (half-Double): 4.1714e-04
Sigmoid output RMSE (hub_float-Double): 4.3506e-04
```

At equal width the two are indistinguishable: identical accuracy, raw output RMSE within 0.3 %.

With the default FP32-equivalent configuration (`EXP_BITS=8`, `MANT_BITS=23`), `hub_float` is compared against the `double` reference:

```
Double precision accuracy: 86.3%
hub_float precision accuracy: 86.3%
Raw output RMSE (hub_float-Double): 1.3820e-06
Sigmoid output RMSE (hub_float-Double): 5.3717e-08
```

The classification decision is unchanged by the substitution.
Building with unbiased rounding (`make clean && make CXX="g++ -DUNBIASED_ROUNDING=1"`) gives the same accuracy (86.3 %) and a raw output RMSE of 1.3740e-06.

The comparison `half` against the 32-bit `hub_float` build would not be meaningful — it credits `hub_float` with 13 extra mantissa bits — and the benchmark has no `float` network to compare the 32-bit build against.

## Using the Neural Network Classes

```cpp
#include "neural.hpp"

// Create a network with float precision
NeuralNetwork<float> network({28*28, 128, 10});  // Input, hidden, output layers

// Load training data
// ...

// Train the network
network.train(training_data, labels, epochs, learning_rate);

// Evaluate on test data
float accuracy = network.evaluate(test_data, test_labels);
```

## Notes

- The implementation supports various activation functions and network architectures
- Memory usage scales with network size and batch size
- For the hub_float implementation, ensure the HUBsim library is properly linked
