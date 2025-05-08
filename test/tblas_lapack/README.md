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

