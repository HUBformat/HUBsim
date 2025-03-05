#include "hub_float.hpp"

#include <cmath>
#include <cstring>  // For std::memcpy
#include <cstdint>  // For uint64_t
#include <sstream>
#include <bitset>
#include <iomanip>

// Enable access to the floating–point environment.
#pragma STDC FENV_ACCESS ON

const double hub_float::maxVal = []() {
    double d;
    uint64_t maxBitsCopy = hub_float::maxBits;
    std::memcpy(&d, &maxBitsCopy, sizeof(d));
    return d;
}();

const double hub_float::minVal = []() {
    double d;
    uint64_t minBitsCopy = hub_float::minBits;
    std::memcpy(&d, &minBitsCopy, sizeof(d));
    return d;
}();
// -------------------------------------------------------------------
// Implementation of hub_float member functions
// -------------------------------------------------------------------

// Default constructor.
hub_float::hub_float() : value(0.0) {}

// Construct from a float.
// Construct from a float by delegating to the double constructor
hub_float::hub_float(float f) : hub_float(static_cast<double>(f)) {}

// Construct from a double.
// If the given double is already on the hub grid, accept it directly;
// otherwise, quantize it
hub_float::hub_float(double d){
    int category = std::fpclassify(d);
    
    if (d > maxVal){
        value = std::numeric_limits<double>::infinity();
    } else if (d < minVal){
        value = -std::numeric_limits<double>::infinity();
    }
    
    if (category == FP_INFINITE || category == FP_ZERO || d == 1.0 || d == -1.0) {
        value = hub_float(d);
        return;
    } else if (category == FP_NAN || category == FP_SUBNORMAL){
    	value = handle_specials(d);
    	return;
    }

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
        // then re-running through quantize
        float f = static_cast<float>(d);
        if (std::fpclassify(f) == FP_NORMAL) {
            value = quantize(static_cast<double>(f));
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
double hub_float::quantize(double d)
{
    double special_result;
    return handle_special_cases(d, special_result) ? special_result : apply_hub_grid(d);
}

// Special case handler
inline bool hub_float::handle_special_cases(double d, double& result) {
    const int category = std::fpclassify(d);
    if (category == FP_INFINITE || category == FP_ZERO || d == 1.0 || d == -1.0) {
        result = d;
        return true;
    }
    if (category == FP_NAN || category == FP_SUBNORMAL) {
        result = handle_specials(d);
        return true;
    }
    return false;
}

// Grid alignment check
inline bool hub_float::is_on_grid(double d) {
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(d));
    return (bits & ((1ULL << SHIFT) - 1)) == HUB_BIT;
}

// Bit manipulation core
inline double hub_float::apply_hub_grid(double d) {
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(d));
    bits = (bits & ~((1ULL << (SHIFT-1)) - 1)) | HUB_BIT;
    std::memcpy(&d, &bits, sizeof(d));
    return d;
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

