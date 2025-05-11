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

## Customization

You can change the number of trials, polynomial degree, and random ranges by editing the variables at the top of `main.cpp`.

## Requirements

- C++11 or newer
- The `hub_float.hpp` header