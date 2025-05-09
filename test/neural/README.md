# Neural Network Implementation and Benchmarking

This directory contains implementations of neural network algorithms and benchmarking tools to compare the precision of standard floating-point and HUB-format floating-point calculations in neural network operations.

## Overview

The neural network implementation provides a templated approach that works with different numeric types, including:
- Standard `float`
- Standard `double` 
- Custom `hub_float` type

The code is designed to compare numerical accuracy, training convergence, and inference performance between these types, with `double` precision used as the reference implementation.

## Files

- `neural.hpp`: Header file with templated class and function declarations for neural networks
- `neural.cpp`: Implementation of neural network algorithms
- `main.cpp`: Benchmark program that compares precision between `float` and `hub_float` types
- `datasets/`: Directory containing sample datasets for training and testing

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
- Tests networks of various sizes and architectures
- Runs multiple training sessions with identical initialization
- Compares `hub_float` against standard `float` using `double` as reference
- Analyzes differences in:
  - Training convergence rate
  - Final accuracy
  - Numerical stability
  - Forward and backward pass precision
- Calculates error statistics (average, maximum, minimum errors)

## Usage

### Running the Benchmark

Compile and run the main program:

```bash
g++ -o neural_benchmark main.cpp neural.cpp -I../../src -std=c++11
./neural_benchmark
```

### Output

The program produces:
1. Console output with summarized results
2. A CSV file containing detailed results for each training session
3. Optional visualizations of training convergence and error distribution

### CSV File Format

The generated CSV includes:
- Network architecture
- Type (`float` or `hub_float`)
- Training epoch
- Training loss
- Validation accuracy
- Test accuracy
- Weight statistics (mean, variance)
- Computation time

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
