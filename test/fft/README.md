# FFT Implementation and Benchmarking

This directory contains an implementation of the Fast Fourier Transform (FFT) algorithm along with benchmarking tools to compare the precision of standard floating-point and HUB-format floating-point calculations.

## Overview

The FFT implementation in this directory provides a templated approach that works with different numeric types, including:
- Standard `float`
- Standard `double`
- Custom `hub_float` type

The code is designed to compare numerical accuracy between these types, with `double` precision used as the reference implementation.

## Files

- `fft.hpp`: Header file with templated function declarations for the FFT algorithm
- `fft.cpp`: Implementation of the FFT algorithm
- `main.cpp`: Benchmark program that compares precision between `float` and `hub_float` types

## FFT Algorithm

The implementation uses the Cooley-Tukey radix-2 decimation-in-time FFT algorithm, which:
1. Rearranges input data using bit-reversal permutation
2. Performs the butterfly operations for computation

The algorithm works in-place, meaning the input arrays are overwritten with the FFT results.

## Attribution

This FFT implementation is based on code from [Lloyd Rochester's Geek Blog](https://lloydrochester.com/post/c/example-fft/#c-implementation-of-the-fft). The Lloyd Rochester's Geek Blog is licensed under a [Creative Commons Attribution 4.0 International License](https://creativecommons.org/licenses/by/4.0/).

The code can also be found in the GitHub repository at https://github.com/lloydroc/arduino_fft, which is worth checking. The GitHub repository is available under the MIT license.

In accordance with these licenses, we acknowledge Lloyd Rochester as the original author and have adapted the code for our specific benchmarking needs.

## Benchmark

The benchmark program:
- Tests FFT on various sizes (powers of 2): 128, 256, 512, 1024, 2048, 4096
- Runs multiple trials (default: 1000) for statistical significance
- Compares `hub_float` against standard `float` using `double` as reference
- Analyzes both real and imaginary parts separately
- Calculates error statistics (average, maximum, minimum, relative errors, and SNR)

## Usage

### Running the Benchmark

Compile and run the main program:

```bash
g++ -o fft_benchmark main.cpp fft.cpp -I../../src -std=c++11
./fft_benchmark
```

### Output

The program produces:
1. Console output with summarized results
2. A CSV file containing detailed results for each trial
3. Optional Mathematica-compatible data files for deeper analysis

### CSV File Format

The generated CSV includes:
- FFT Size
- Type (`float` or `hub_float`)
- Component (real or imaginary)
- Trial number
- Average error
- Maximum error
- Minimum error
- Relative error
- Variance
- SNR (dB)

## Using the FFT Functions

```cpp
#include "fft.hpp"

// Example with float data
unsigned int N = 1024; // Must be a power of 2
std::vector<float> real_part(N), imag_part(N);

// Fill with your data...

// Perform FFT in-place
fft(real_part.data(), imag_part.data(), N);

// real_part and imag_part now contain the FFT result
```

## Notes

- The implementation requires the input length to be a power of 2
- Results are not normalized (divide by √N if normalization is needed)
- For the hub_float implementation, ensure the HUBsim library is properly linked