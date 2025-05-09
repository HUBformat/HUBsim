/*
    File: hub_float.cpp
    Implementation of the hub_float class.
*/
#include "hub_float.hpp"

#include <cmath>
#include <cstring>  // For std::memcpy
#include <cstdint>  // For uint64_t
#include <sstream>
#include <bitset>
#include <iomanip>

// Enable access to the floating–point environment.
#pragma STDC FENV_ACCESS ON

/*
    Variable: maxVal
    The maximum representable value for hub_float.
*/
const double hub_float::maxVal = []() {
    double d;
    uint64_t maxBitsCopy = hub_float::maxBits;
    std::memcpy(&d, &maxBitsCopy, sizeof(d));
    return d;
}();

/*
    Variable: minVal
    The minimum representable value for hub_float.
*/
const double hub_float::minVal = []() {
    double d;
    uint64_t minBitsCopy = hub_float::minBits;
    std::memcpy(&d, &minBitsCopy, sizeof(d));
    return d;
}();

/*
    Variable: lowestVal
    The lowest representable absolute value for hub_float.
*/
const double hub_float::lowestVal = []() {
    double d;
    uint64_t minPosBitsCopy = hub_float::minPosBits;
    std::memcpy(&d, &minPosBitsCopy, sizeof(d));
    return d;
}();

// -------------------------------------------------------------------
// Implementation of hub_float member functions
// -------------------------------------------------------------------

/*
    Function: hub_float
    Default constructor. Initializes the value to zero.
*/
hub_float::hub_float() : value(0.0) {}

/*
    Function: hub_float
    Constructor that converts a float to a hub_float.

    Parameters:
        f - The float value to convert.
*/
hub_float::hub_float(float f) : hub_float(static_cast<double>(f)) {}

/*
    Function: hub_float
    Constructor that converts a double to a hub_float.

    Parameters:
        d - The double value to convert.
*/
hub_float::hub_float(double d) {
    int category = std::fpclassify(d);
    
    if (category == FP_INFINITE || category == FP_ZERO || d == 1.0 || d == -1.0) {
        value = d;
        return;
    } else if (category == FP_NAN || (std::abs(d) < lowestVal && d != 0.0 && d != -0.0)){
    	value = handle_specials(d);
    	return;
    }

    if (d > maxVal){
        value = std::numeric_limits<double>::infinity();
    } else if (d < minVal){
        value = -std::numeric_limits<double>::infinity();
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

/*
    Function: hub_float
    Constructor that creates a hub_float from a raw binary representation.

    Parameters:
        binary_value - The raw binary value representing the sign, exponent, and mantissa.
*/
hub_float::hub_float(uint32_t binary_value) {
    // Extract components
    int sign = (binary_value >> (EXP_BITS + MANT_BITS)) & 0x1;
    uint64_t custom_exp = (binary_value >> MANT_BITS) & ((1 << EXP_BITS) - 1);
    uint64_t custom_frac = binary_value & ((1 << MANT_BITS) - 1);
    
    // Handle special cases
    if (custom_exp == 0 && custom_frac == 0) {
        // Zero: (Sx, 0, 0) - both exponent and fraction must be zero
        value = sign ? -0.0 : 0.0;
        return;
    }
    
    if (custom_exp == (1 << (EXP_BITS - 1)) && custom_frac == 0) {
        // One: (Sx, 2^(n_exp-1), 0) - specific exponent value and fraction must be zero
        value = sign ? -1.0 : 1.0;
        return;
    }
    
    if (custom_exp == ((1 << EXP_BITS) - 1) && custom_frac == ((1ULL << MANT_BITS) - 1)) {
        // Infinity: (Sx, 2^(n_exp)-1, 2^(n_m)-1) - both exponent and fraction must be all ones
        value = sign ? -std::numeric_limits<double>::infinity() : 
                       std::numeric_limits<double>::infinity();
        return;
    }
    
    // Convert to double
    // 1. Adjust the exponent from custom bias to IEEE double bias
    int double_exp = custom_exp + BIAS_DIFF;
    
    // 2. Prepare the mantissa with the implicit HUB bit
    uint64_t double_frac = (custom_frac << SHIFT) | HUB_BIT;
    
    // 3. Assemble the IEEE double bits
    uint64_t double_bits = (static_cast<uint64_t>(sign) << 63) | 
                          (static_cast<uint64_t>(double_exp) << 52) | 
                          double_frac;
    
    // 4. Convert bits to double
    std::memcpy(&value, &double_bits, sizeof(value));
}

/*
   Function: operator double
   Converts a hub_float to a double.

   Returns:
       The internal value as a double.
*/
hub_float::operator double() const {
    return value;
}

/*
   Function: quantize
   Quantizes a double to the nearest point on the hub grid.

   Parameters:
       d - The double value to quantize.

   Returns:
       The quantized double value.
*/
double hub_float::quantize(double d)
{
    double special_result;
    return handle_special_cases(d, special_result) ? special_result : apply_hub_grid(d);
}

/*
   Function: handle_special_cases
   Handles special floating-point cases like NaN and infinity.

   Parameters:
       d - The input double value.
       result - Output result for special cases.

   Returns:
       True if a special case was handled; false otherwise.
*/
inline bool hub_float::handle_special_cases(double d, double& result) {
    const int category = std::fpclassify(d);
    if (category == FP_INFINITE || category == FP_ZERO || d == 1.0 || d == -1.0) {
        result = d;
        return true;
    }
    if (category == FP_NAN || (std::abs(d) < lowestVal && d != 0.0 && d != -0.0))  {
        result = handle_specials(d);
        return true;
    }
    return false;
}

/*
    Function: is_on_grid
    Checks if a double value is already on the hub grid.

    Parameters:
        d - The double value to check.

    Returns:
        True if the value is on the grid, false otherwise.
*/
inline bool hub_float::is_on_grid(double d) {
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(d));
    return (bits & ((1ULL << SHIFT) - 1)) == HUB_BIT;
}

/*
    Function: apply_hub_grid
    Applies the hub grid to a double value.

    Parameters:
        d - The double value to quantize.

    Returns:
        The quantized double value.
*/
inline double hub_float::apply_hub_grid(double d) {
    uint64_t bits;


    std::memcpy(&bits, &d, sizeof(d));
    bits = (bits & ~((1ULL << (SHIFT-1)) - 1)) | HUB_BIT;
    std::memcpy(&d, &bits, sizeof(d));

    if (d > maxVal){
        return std::numeric_limits<double>::infinity();
    } else if (d < minVal){
        return -std::numeric_limits<double>::infinity();
    }
    
    return d;
}

/*
    Function: handle_specials
    Handles special values like NaN and subnormal numbers.

    Parameters:
        d - The input double value.

    Returns:
        The processed result for special values.
*/
double hub_float::handle_specials(double d) {
    if (std::fpclassify(d) == FP_NAN) {
        return std::signbit(d) ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
    } else if (std::abs(d) < lowestVal && d != 0.0 && d != -0.0) {
        return std::signbit(d) ? -0.0 : 0.0;
    } else {
        return d;
    }
}

/*
    Function: operator+
    Adds two hub_float values.

    Parameters:
        other - The hub_float to add.

    Returns:
        A new hub_float containing the sum.
*/
hub_float hub_float::operator+(const hub_float &other) const {
    return quantize(this->value + other.value);
}

/*
    Function: operator-
    Subtracts one hub_float from another.

    Parameters:
        other - The hub_float to subtract.

    Returns:
        A new hub_float containing the difference.
*/
hub_float hub_float::operator-(const hub_float &other) const {
    return quantize(this->value - other.value);
}

/*
    Function: operator*
    Multiplies two hub_float values.

    Parameters:
        other - The hub_float to multiply by.

    Returns:
        A new hub_float containing the product.
*/
hub_float hub_float::operator*(const hub_float &other) const {
    return quantize(this->value * other.value);
}

/*
    Function: operator/
    Divides one hub_float by another.

    Parameters:
        other - The hub_float to divide by.

    Returns:
        A new hub_float containing the quotient.
*/
hub_float hub_float::operator/(const hub_float &other) const {
    return quantize(this->value / other.value);
}

/*
    Function: operator+=
    Adds another hub_float to this one and assigns the result.

    Parameters:
        other - The hub_float to add.

    Returns:
        A reference to this object after addition.
*/
hub_float& hub_float::operator+=(const hub_float &other) {
    *this = *this + other;
    return *this;
}

/*
    Function: operator-=
    Subtracts another hub_float from this one and assigns the result.

    Parameters:
        other - The hub_float to subtract.

    Returns:
        A reference to this object after subtraction.
*/
hub_float& hub_float::operator-=(const hub_float &other) {
    *this = *this - other;
    return *this;
}

/*
    Function: operator*=
    Multiplies this object by another hub_float and assigns the result.

    Parameters:
        other - The hub_float to multiply by.

    Returns:
        A reference to this object after multiplication.
*/
hub_float& hub_float::operator*=(const hub_float &other) {
    *this = *this * other;
    return *this;
}

/*
   Function: operator/=
   Divides this object by another hub_float and assigns the result.

   Parameters:
       other - The hub_float to divide by.

   Returns:
       A reference to this object after division.
*/
hub_float& hub_float::operator/=(const hub_float &other) {
    *this = *this / other;
    return *this;
}

/*
   Function: extractBitFields
   Extracts the bit fields from the internal representation of a hub_float.

   Returns:
       A BitFields structure containing the extracted fields (sign, exponent, fraction).
*/
hub_float::BitFields hub_float::extractBitFields() const {
    hub_float::BitFields fields;
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    
    // Extract components
    fields.sign = (bits >> 63) & 1;
    int double_exp = static_cast<int>((bits >> 52) & 0x7FF);
    fields.fraction = bits & ((1ULL << 52) - 1ULL);

    if (value == 0.0 || value == -0.0) {
        fields.custom_exp = 0;
        fields.custom_frac = 0;
        fields.custom_frac_with_hub = 0;
        return fields;
    }

    if (value == 1.0 || value == -1.0) {
        // One: exponent is 2^(n_exp-1) and significand is 0
        fields.custom_exp = (1 << (EXP_BITS - 1));
        fields.custom_frac = 0;
        fields.custom_frac_with_hub = 0;
        return fields;
    }

    if (std::isinf(value)) {
        // Infinity: all 1s for exponent and significand
        fields.custom_exp = (1 << EXP_BITS) - 1;
        fields.custom_frac = (1ULL << MANT_BITS) - 1;
        fields.custom_frac_with_hub = ((1ULL << (MANT_BITS + 1)) - 1);
        return fields;
    }
    
    // Convert IEEE-754 double exponent to custom exponent
    fields.custom_exp = double_exp - BIAS_DIFF;
    
    // Extract custom fraction bits (without HUB bit)
    fields.custom_frac = (fields.fraction >> SHIFT) & ((1ULL << MANT_BITS) - 1);
    
    // For binary string representation, include HUB bit
    fields.custom_frac_with_hub = fields.fraction >> (SHIFT - 1);
    
    return fields;
}

/*
   Function: toBinaryString
   Converts a hub_float to its binary string representation in the format S|EEEEEEEE|MMMMMMMMMMMMMMMMMMMMMMMM.

   Returns:
       A string representing the binary format of the number.
*/
std::string hub_float::toBinaryString() const {
    BitFields fields = extractBitFields();
    
    // Build the string: sign, exponent (EXP_BITS bits), fraction (MANT_BITS+1 bits)
    std::ostringstream oss;
    oss << fields.sign << '|'
        << std::bitset<EXP_BITS>(static_cast<unsigned long long>(fields.custom_exp))
        << '|'
        << std::bitset<MANT_BITS+1>(fields.custom_frac_with_hub);

    return oss.str();
}

/*
   Function: toHexString
   Converts a hub_float to its hexadecimal string representation in a compact format.

   Returns:
       A string containing the hexadecimal representation of the number prefixed with "0x".
*/
std::string hub_float::toHexString() const {
    hub_float::BitFields fields = extractBitFields();
    
    // Pack components
    const int total_bits = 1 + EXP_BITS + MANT_BITS;
    const int hex_digits = (total_bits + 3) / 4; // Ceiling division
    // Modify packing to use only required bits
    const uint64_t packed = ((fields.sign & 0x1) << (EXP_BITS + MANT_BITS)) |
    ((fields.custom_exp & ((1 << EXP_BITS)-1)) << MANT_BITS) |
    (fields.custom_frac & ((1 << MANT_BITS)-1));

    // Format hex string
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase 
        << std::setw(hex_digits) << std::setfill('0') << packed;
    
    return oss.str();
}


// -------------------------------------------------------------------
// Non-member functions
// -------------------------------------------------------------------

/*
   Function: operator<<
   Outputs a human-readable representation of a hub_float to an output stream.

   Parameters:
       os - The output stream.
       hf - The hub_float object to output.

   Returns:
       A reference to the output stream after writing.
*/
std::ostream& operator<<(std::ostream &os, const hub_float &hf) {
    os << hf.value;
    return os;
}

/*
   Function: sqrt
   Computes the square root of a given hub_float value, ensuring it conforms to the custom grid representation.

   Parameters:
       x - The input value as a hub_float.

   Returns:
       A new quantized result as a square root in grid form.
*/
hub_float sqrt(const hub_float &x) {
    return hub_float::quantize(std::sqrt(static_cast<double>(x)));
}


/*
   Function: operator"" _hb
   User-defined literal for creating a `hub_float` from a literal value with `_hb` suffix.

   Parameters:
       d - The long double literal value being converted into `hub`.

  Return 
	   An equivalent `Hub_Float` instance 
*/       
hub_float operator"" _hb(long double d) {
    return hub_float(static_cast<double>(d));
}

// -------------------------------------------------------------------
// End of hub_float.cpp
// -------------------------------------------------------------------

