# HUB-Float Simulation Library

A C++ implementation of the Half-Unit-Biased (HUB) floating-point number representation format.

## Overview

HUB (Half-Unit-Biased) is an emerging number representation format that shifts the representation line of binary numbers by half unit in the last place (ULP). The key feature of HUB format is that rounding to nearest is achieved simply by truncation, which eliminates the need for complex rounding logic and prevents carry propagation during rounding operations.

This library provides a C++ implementation of the HUB format with configurable exponent and mantissa bit widths, defaulting to IEEE-754 single precision parameters (8-bit exponent, 23-bit mantissa).

## Features

- **Configurable Precision**
- **Standard Arithmetic Operations**
- **FMA Emulation**
- **Special Value Handling**
- **Conversion Support**
- **Diagnostic Functions**

## Usage

### Basic Operations

```cpp
#include "hub_float.hpp"

int main() {
    // Create HUB float values
    hub_float a = 3.14_hb;        // Using literal
    hub_float b(2.71);            // From double
    hub_float c = a + b;          // Addition
    
    // Arithmetic operations
    hub_float result = a * b + c; // Multiplication and addition
    
    // Display binary representation
    std::cout << "Binary: " << result.toBinaryString() << std::endl;
    std::cout << "Hex: " << result.toHexString() << std::endl;
    
    return 0;
}
```

## Key Characteristics

- **Implicit Least Significant Bit (ILSB)**: In HUB format, the least significant bit is always 1 and is implicit
- **Simplified Rounding**: Rounding to nearest is achieved by simple truncation
- **Hardware Efficiency**: Eliminates carry propagation during rounding operations
- **Same Storage Requirements**: Requires the same storage space as conventional formats with equivalent precision

## Implementation Details

The implementation uses a double as the internal storage format, with bit manipulation to ensure values conform to the HUB grid. Key implementation aspects include:

- Quantization to the HUB grid via bit manipulation
- Special case handling for values like infinity, NaN, and subnormals
- Proper extraction of bit fields for string representations
- Support for all basic arithmetic operations
- Detection and correction of double rounding in float32 FMA emulation (EXP_BITS=8, MANT_BITS=23) for accurate single-precision results

## References

For more information about the HUB format, refer to:

- J. Hormigo and J. Villalba, "New formats for computing with real-numbers under round-to-nearest," in IEEE Transactions on Computers
- J. Hormigo and J. Villalba, "Measuring Improvement When Using HUB Formats to Implement Floating-Point Systems under Round-to-Nearest," in IEEE Transactions on Very Large Scale Integration (VLSI) Systems

![Logos of the Spanish Government, European Union NextGenerationEU, Spanish Recovery and Resilience Plans, and Spanish State Research Agency.](https://github.com/HUBformat/.github/raw/main/profile/res/MICIU+NextG+PRTR+AEI.svg)&nbsp;


