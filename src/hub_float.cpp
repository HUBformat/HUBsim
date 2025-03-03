#include "hub_float.hpp"

#include <cmath>
#include <cfenv>    // For fesetround, fegetround, FE_DOWNWARD
#include <cstring>  // For std::memcpy
#include <cstdint>  // For uint64_t
#include <sstream>
#include <bitset>
#include <iomanip>


#ifndef EXP_BITS
#define EXP_BITS 8 // Double: 11
#endif

#ifndef MANT_BITS
#define MANT_BITS 23 // Double: 52
#endif

// SHIFT = number of low-order bits in the double's mantissa that we will force/clear.
static constexpr int SHIFT = 52 - MANT_BITS;

// The bit (within bits 0..51 of a double's mantissa) we use to emulate the
// "implicit leading 1" in a normalized IEEE style. For single precision (23 mantissa),
// SHIFT = 29, so HUB_BIT = 1 << 28, matching the old code.
static const uint64_t HUB_BIT = (1ULL << (SHIFT - 1));

// The difference in biases: double’s bias = 1023, custom bias = (1<<(EXP_BITS-1)) - 1.
//static const int CUSTOM_BIAS = (1 << (EXP_BITS - 1)) - 1;
static const int CUSTOM_BIAS = (1 << (EXP_BITS - 1)) - 1;
static const int BIAS_DIFF   = 1023 - CUSTOM_BIAS; 
// For single precision (EXP_BITS=8, MANT_BITS=23), BIAS_DIFF = 1023 - 127 = 896.

// Enable access to the floating–point environment.
#pragma STDC FENV_ACCESS ON

// -------------------------------------------------------------------
// Implementation of hub_float member functions
// -------------------------------------------------------------------

// Default constructor.
hub_float::hub_float() : value(0.0) {}

// Construct from a float.
// For normalized floats, convert to double exactly then force the extra bit.
hub_float::hub_float(float f)
{
    // For zeros, infinities, subnormals, etc., just store as double
    if (f == 0.0f || !std::isfinite(f) || std::fpclassify(f) != FP_NORMAL) {
        value = static_cast<double>(f);
    } else {
        // Convert to double exactly, then force it onto our grid
        double d = static_cast<double>(f);
        value = float_to_hub(d);
    }
}

// Construct from a double.
// If the given double is already on the hub grid, accept it directly;
// otherwise, quantize it by converting to float (which rounds to nearest)
// and then using the float constructor.
hub_float::hub_float(double d){
    d = handle_specials(d);

    // Check if 'd' is already on our custom grid
    // We'll do that by re-applying the mask logic and comparing bits.
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(d));

    // If it's on the grid, the lower SHIFT bits must be exactly
    // HUB_BIT in that block (assuming normal).
    const uint64_t mask = ((1ULL << SHIFT) - 1ULL);  // SHIFT lower bits
    const uint64_t desired = HUB_BIT;                // we want exactly this pattern

    if ((bits & mask) == desired) {
        // Already on the hub grid
        value = d;
    } else {
        // Not on the grid, quantize by converting to float (round to nearest)
        // then re-running through float_to_hub
        float f = static_cast<float>(d);
        if (std::fpclassify(f) == FP_NORMAL) {
            value = float_to_hub(static_cast<double>(f));
        } else {
            value = static_cast<double>(f);
        }
    }
}

// Implicit conversion to double.
hub_float::operator double() const {
    return value;
}

// Helper: Force the extra (24th) significand bit in the double.
double hub_float::float_to_hub(double d)
{
    // If not normal, just return it directly
    if (!std::isfinite(d) || std::fpclassify(d) != FP_NORMAL) {
        return d;
    }

    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(d));

    // 1) Zero out the lower (SHIFT - 1) bits
    // 2) Force HUB_BIT (the "implicit" bit)
    const uint64_t lowerMask = (1ULL << (SHIFT - 1)) - 1ULL; // bits below HUB_BIT
    // Clear out everything below SHIFT - 1
    bits &= ~lowerMask;
    // Now ensure HUB_BIT is set
    bits |= HUB_BIT;

    // Put it back
    std::memcpy(&d, &bits, sizeof(d));
    return d;
}

// Helper: Quantize a double result using truncation.
hub_float hub_float::quantize(double d)
{
    if (d == 0.0 || !std::isfinite(d) || std::fpclassify(d) != FP_NORMAL) {
        return hub_float(d);
    }

    // We do the same “mask and set” logic
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(d));

    // Clear out the lower SHIFT-1 bits
    const uint64_t lowerMask = (1ULL << (SHIFT - 1)) - 1ULL;
    bits &= ~lowerMask;

    // Force the "implicit" bit
    bits |= HUB_BIT;

    // Convert bits back to double
    double truncated;
    std::memcpy(&truncated, &bits, sizeof(truncated));

    return hub_float(truncated);
}

double hub_float::handle_specials(double d) {
    switch (std::fpclassify(d)) {
        case FP_NAN:
            return std::signbit(d) ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
        case FP_SUBNORMAL:
            return std::signbit(d) ? -0.0 : 0.0;
        default:
            return d;
    }
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
std::string hub_float::toBinaryString() const 
{
    // Grab the raw 64-bit representation of the double
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));

    // 1) Sign bit: bit 63
    int sign = (bits >> 63) & 1;

    // 2) 11-bit exponent in bits [52..62]
    int double_exp = static_cast<int>((bits >> 52) & 0x7FF);

    // 3) Fraction (mantissa) bits: bits [0..51]
    uint64_t fraction = bits & ((1ULL << 52) - 1ULL);

    // Convert IEEE-754 double exponent to our custom exponent
    int custom_exp = double_exp - BIAS_DIFF;  

    // SHIFT = 52 - MANT_BITS
    // We want (MANT_BITS+1) bits out of the fraction field
    // => shift right by (SHIFT - 1)
    // Example: MANT_BITS=5 => SHIFT=47 => SHIFT-1=46 => fraction>>46 leaves 6 bits
    uint64_t custom_frac = fraction >> (SHIFT - 1); 

    // Build the string: sign, exponent (EXP_BITS bits), fraction (MANT_BITS+1 bits)
    std::ostringstream oss;
    oss << sign << '|'
        << std::bitset<EXP_BITS>(static_cast<unsigned long long>(custom_exp))
        << '|'
        << std::bitset<MANT_BITS+1>(custom_frac);

    return oss.str();
}

std::string hub_float::toHexString32() const
{
    // First clear the hub bit and convert to float
    double temp = value;
    uint64_t bits64;
    std::memcpy(&bits64, &temp, sizeof(bits64));
    
    // Clear bit 28 (the ILS bit)
    bits64 &= ~(1ULL << 28);
    std::memcpy(&temp, &bits64, sizeof(temp));
    
    // Convert to float
    float f = static_cast<float>(temp);
    
    // Now get the bits of the float
    uint32_t bits32;
    std::memcpy(&bits32, &f, sizeof(bits32));

    // Format as an 8-digit hex string
    std::ostringstream oss;
    oss << std::hex << std::setw(8) << std::setfill('0') << bits32;
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

hub_float sqrt(const hub_float &x)
{
    return hub_float::quantize(std::sqrt(static_cast<double>(x)));
}


// User–defined literal for hub_float.
hub_float operator"" _hb(long double d) {
    return hub_float(static_cast<double>(d));
}


// -------------------------------------------------------------------
// End of hub_float.cpp
// -------------------------------------------------------------------

