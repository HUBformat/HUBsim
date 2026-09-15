# hub_float Demo: Horner's Rule Accuracy Test

This subfolder contains a demonstration comparing the numerical accuracy of the custom `hub_float` floating-point type versus standard `float` using Horner's Rule for polynomial evaluation.

## Contents

- `main.cpp` &mdash; Source code for the demo.

## What It Does

- Randomly generates polynomials and evaluation points.
- Computes the polynomial value using:
  - Standard `float`
  - Custom `hub_float`
  - Reference `double`
- Measures and compares the error of `float` and `hub_float` against `double`.
- Summarizes which type is more accurate across many random cases.

## Usage

Build from the repository root with `make`; the binary lands in `bin/`:

```bash
make
./bin/horner        # seed 42
./bin/horner 7      # any other seed
```

The optional seed makes a run reproducible and lets independent repetitions be run; without it the program uses 42.

## Customization

You can change the number of trials, polynomial degree, and random ranges by editing the variables at the top of `main.cpp`.

## Results

The two error columns are the program's `Average float error` and `Average hub_float error` lines, which it prints to three significant digits; the last column is its `Ratio hub_error/float_error` line.

| Seed | Average `float` error | Average `hub_float` error | Ratio `hub_float` / `float` |
|---:|---:|---:|---:|
| 1 | 1.21e+04 | 1.20e+04 | 0.9925 |
| 2 | 1.21e+04 | 1.21e+04 | 1.0045 |
| 3 | 1.20e+04 | 1.20e+04 | 1.0024 |
| 4 | 1.19e+04 | 1.22e+04 | 1.0259 |
| 5 | 1.18e+04 | 1.19e+04 | 1.0052 |
| 6 | 1.20e+04 | 1.18e+04 | 0.9851 |
| 7 | 1.17e+04 | 1.19e+04 | 1.0100 |
| 8 | 1.21e+04 | 1.22e+04 | 1.0090 |
| 9 | 1.21e+04 | 1.20e+04 | 0.9891 |
| 10 | 1.20e+04 | 1.20e+04 | 1.0003 |
| **mean ± sd** | | | **1.0024 ± 0.0118** |

The ratio stays between 0.9851 and 1.0259 and averages 1.0024: at the same number of stored mantissa bits the two types give the same accuracy, and the spread between seeds is sampling variation.
The absolute errors look large only because the polynomial values are: with coefficients up to 100 and evaluation points up to 10, a degree-10 polynomial can reach the order of 10¹².

Building with unbiased rounding (`make clean && make CXX="g++ -DUNBIASED_ROUNDING=1"`) gives the same ratio on every seed to within 0.0001 (mean 1.0024 ± 0.0118).
The rounding mode only changes the result of an operation that lands exactly between two representable values, so identical results mean such ties have no measurable effect on this workload.

## Requirements

- A C++17 compiler (the Makefile uses `-std=c++17`)
- `src/hub_float.hpp` and `src/hub_float.cpp`