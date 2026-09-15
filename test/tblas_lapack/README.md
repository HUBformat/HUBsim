# tblas_lapack

This directory contains implementations of BLAS (Basic Linear Algebra Subprograms) and LAPACK (Linear Algebra PACKage) routines used in the HUBsim project.

## Attribution

The code in this folder is derived from:

- BLAS and LAPACK implementations from the University of Tennessee
- Code taken from https://github.com/victorliu/RNP

## Modifications

The implementations in this folder have been:
- Changed for the specific use of HUBsim

## Functionality

This code is primarily used for benchmarking numerical precision in HUBsim. We run LAPACK routines to establish baseline accuracy measurements for our computational methods, ensuring reliable results across different network scales and configurations.

The benchmarking helps validate the numerical stability and accuracy of HUBsim

## Benchmark (`main.cpp`)

`main.cpp` solves random, diagonally dominant linear systems with `RNP::LinearSolve` (LU with partial pivoting) in `double`, `float` and `hub_float`, and compares the `float` and `hub_float` solutions against the `double` one.
It asks which test to run; option 2 is the multi-size sweep, from 10×10 to 100×100, with up to 50 independent systems per size and an early stop once the SNR has stabilised:

```bash
make
echo 2 | ./bin/tblas_lapack
```

Results go to a timestamped `tblas_results_*` directory (`results_summary.csv` plus the matrices of each trial).
Per-solve tracing is available by building with `-DVERBOSE_SOLVE=1`.

### Results


```
===== SUMMARY FOR MATRIX SIZE 10x10 =====
Trials completed: 50
Float average SNR: 140.252 dB
Hub_float average SNR: 140.419 dB
SNR improvement in dB: 0.167318 dB

===== SUMMARY FOR MATRIX SIZE 100x100 =====
Trials completed: 22
Float average SNR: 129.75 dB
Hub_float average SNR: 129.733 dB
SNR improvement in dB: -0.0166198 dB
```

`float` and `hub_float` agree to within 0.4 dB at every size from 10×10 to 100×100.

