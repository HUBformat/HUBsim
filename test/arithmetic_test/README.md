# Arithmetic Test Suite for HUB-Float

This subfolder contains the test suite for generating testbenchs for the arithmetic operations of the HUB-Float library. The tests are designed to ensure correctness and robustness of the custom floating-point implementation under various scenarios, including edge cases and random sampling.

## Overview

The test suite evaluates the following operations on the HUB-Float representation:

- **Addition**
- **Multiplication**
- **Division**
- **Square Root**

Each operation is tested exhaustively (when feasible) or via random sampling, depending on the configuration.

## Folder Structure

- **`main.cpp`**: Entry point for running the test suite. It initializes and executes tests for all supported operations.
- **`operation_tester.hpp`** and **`operation_tester.cpp`**: Define the `OperationTester` class and its implementation for testing unary and binary operations.
- **`utils.hpp`** and **`utils.cpp`**: Provide utility functions for file handling, progress display, and result visualization.
- **`test_config.hpp`**: Contains configuration constants for controlling test behavior, such as the maximum number of exhaustive tests and random sample size.

## Key Features

- **Exhaustive Testing**: Tests all possible combinations of inputs when the total number of combinations is within a feasible range.
- **Random Sampling**: For larger input spaces, tests a random subset of combinations to ensure coverage without excessive computation.
- **Special Case Handling**: Includes tests for edge cases like zero, infinity, NaN, and subnormal values.
- **Detailed Output**: Optionally displays detailed results for each calculation, including hexadecimal and binary representations.

## Configuration

The behavior of the test suite can be customized via the `test_config.hpp` file:

- `MAX_EXHAUSTIVE_TESTS`: Maximum number of combinations for exhaustive testing.
- `RANDOM_SAMPLE_SIZE`: Number of samples for random testing.
- `RANDOM_SEED`: Seed for the random number generator to ensure reproducibility.
- `SHOW_DETAILED_OUTPUT`: Whether to display detailed output for each calculation.