#include "hub_float.hpp"

#include <cmath>
#include <cfenv>    // For fesetround, fegetround, FE_DOWNWARD
#include <cstring>  // For std::memcpy
#include <cstdint>  // For uint64_t
#include <sstream>
#include <bitset>


// Enable access to the floating–point environment.
#pragma STDC FENV_ACCESS ON

// -------------------------------------------------------------------
// Implementation of hub_float member functions
// -------------------------------------------------------------------

// Default constructor.
hub_float::hub_float() : value(0.0) {}

// Construct from a float.
// For normalized floats, convert to double exactly then force the extra bit.
hub_float::hub_float(float f) {
    if (f == 0.0f || !std::isfinite(f) || std::fpclassify(f) != FP_NORMAL) {
        value = static_cast<double>(f);
        return;
    }
    double d = static_cast<double>(f);
    value = float_to_hub(d);
}

// Construct from a double.
// If the given double is already on the hub grid, accept it directly;
// otherwise, quantize it by converting to float (which rounds to nearest)
// and then using the float constructor.
hub_float::hub_float(double d) {
    if (d == 0.0 || !std::isfinite(d) || std::fpclassify(d) != FP_NORMAL) {
        value = d;
        return;
    }
    
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(d));
    
    // The hub grid expects that, for a normalized value,
    // the lower 29 bits of the double's significand equal 0x10000000.
    const uint64_t mask29 = (1ULL << 29) - 1; // 29 ones.
    uint64_t lower29 = bits & mask29;
    
    if (lower29 == (1ULL << 28)) {
        // Already on the hub grid.
        value = d;
    } else {
        // Not on the grid; quantize via conversion to float.
        hub_float tmp(static_cast<float>(d));
        value = tmp.value;
    }
}

// Implicit conversion to double.
hub_float::operator double() const {
    return value;
}

// Helper: Force the extra (24th) significand bit in the double.
double hub_float::float_to_hub(double d) {
    // For non-normal numbers, just return d.
    float f = static_cast<float>(d);
    if (f == 0.0f || !std::isfinite(f) || std::fpclassify(f) != FP_NORMAL)
        return d;
    
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(d));
    
    // The extra bit (24th bit) corresponds to bit 28 in the double's fraction.
    const uint64_t hub_mask = (1ULL << 28);
    
    // Work on the magnitude only (preserve the sign).
    if (bits & (1ULL << 63)) { // negative number
        uint64_t mag = bits & ~(1ULL << 63);
        mag |= hub_mask;
        bits = mag | (1ULL << 63);
    } else {
        bits |= hub_mask;
    }
    
    std::memcpy(&d, &bits, sizeof(d));
    return d;
}

// Helper: Quantize a double result using FE_DOWNWARD rounding.
// This function temporarily sets the rounding mode to FE_DOWNWARD for the conversion.
hub_float hub_float::quantize(double d) {
    // If zero, subnormal, NaN, or Inf, just store directly
    if (d == 0.0 || !std::isfinite(d) || std::fpclassify(d) != FP_NORMAL) {
        return hub_float(d);
    }

    // Convert the double to its raw bits using memcpy
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(double));

    // Clear out the lower 28 bits in the fraction field
    bits &= ~((1ULL << 28) - 1ULL);

    // Force bit 28 (the "24th custom mantissa bit") to 1
    bits |= (1ULL << 28);

    // Convert bits back to double using memcpy
    double truncated;
    std::memcpy(&truncated, &bits, sizeof(double));
    return hub_float(truncated);
}

hub_float hub_float::operator+(const hub_float &other) const {
    return quantize(this->value + other.value);
}

hub_float hub_float::operator-(const hub_float &other) const {
    return quantize(this->value - other.value);
}

hub_float hub_float::operator*(const hub_float &other) const {
    return quantize(this->value * other.value);
}

hub_float hub_float::operator/(const hub_float &other) const {
    return quantize(this->value / other.value);
}

hub_float& hub_float::operator+=(const hub_float &other) {
    *this = *this + other;
    return *this;
}

hub_float& hub_float::operator-=(const hub_float &other) {
    *this = *this - other;
    return *this;
}

hub_float& hub_float::operator*=(const hub_float &other) {
    *this = *this * other;
    return *this;
}

hub_float& hub_float::operator/=(const hub_float &other) {
    *this = *this / other;
    return *this;
}

// This function extracts the custom hub_float representation in the following format:
//
//    S|EEEEEEEE|MMMMMMMMMMMMMMMMMMMMMMMM
//
std::string hub_float::toBinaryString() const {
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(value));
    
    // For normalized numbers, extract the fields.
    // Sign: bit 63.
    int sign = (bits >> 63) & 0x1;
    
    // Double exponent: bits 62..52 (11 bits).
    int double_exp = (bits >> 52) & 0x7FF;
    // Compute the float exponent (8-bit) from the double exponent.
    int float_exp = double_exp - 896; // since 1023 - 127 = 896.
    
    // Extract custom mantissa: we want the 24 bits starting from bit 28.
    uint32_t custom_mantissa = (bits >> 28) & 0xFFFFFF; // 24 bits.
    
    std::ostringstream oss;
    oss << sign << "|" 
        << std::bitset<8>(static_cast<unsigned>(float_exp)) << "|" 
        << std::bitset<24>(custom_mantissa);
    return oss.str();
}

// -------------------------------------------------------------------
// Non-member functions
// -------------------------------------------------------------------

// Overload the stream insertion operator.
std::ostream& operator<<(std::ostream &os, const hub_float &hf) {
    os << hf.value;
    return os;
}

// User–defined literal for hub_float.
hub_float operator"" _hb(long double d) {
    return hub_float(static_cast<double>(d));
}

// -------------------------------------------------------------------
// End of hub_float.cpp
// -------------------------------------------------------------------

