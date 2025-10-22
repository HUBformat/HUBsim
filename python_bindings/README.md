# HubFloat Python Bindings

This directory contains Python bindings for the `hub_float` class using pybind11.

## Building

To build the Python extension:

```bash
python3 setup.py build_ext --inplace
```

## Usage

```python
import hub_float as hf

# Create hub_float numbers
a = hf.HubFloat(3.14159)
b = hf.HubFloat(2.71828)

# Basic arithmetic
result = a + b * 2.0  # Mixed with Python floats!
print(f"Result: {result}")

# Special functions
sqrt_a = hf.sqrt(a)
fused = hf.fma(a, b, 1.0)  # (a*b + 1.0) with single rounding

# Inspect internal representation
bits = a.extract_bit_fields()
print(f"Binary: {a.to_binary_string()}")
print(f"Hex: {a.to_hex_string()}")
```

## Features

- **Constructors**: From `int`, `float`, `double`, or raw binary
- **Arithmetic**: `+`, `-`, `*`, `/` with other HubFloat or Python numbers
- **Comparisons**: `==`, `!=`, `<`, `<=`, `>`, `>=`
- **Conversions**: `float(hub_float_obj)` works naturally
- **Utilities**: 
  - `extract_bit_fields()` - Get internal bit structure
  - `to_binary_string()` - Binary representation 
  - `to_hex_string()` - Hexadecimal representation
- **Math Functions**: `sqrt()`, `fma()`
- **Constants**: `EXP_BITS`, `MANT_BITS`

## Requirements

- Python 3.6+
- pybind11
- C++17 compiler
