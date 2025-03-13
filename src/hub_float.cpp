/**
 * @file hub_float.cpp
 * @brief Implementation of the hub_float class
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

/**
 * @brief Initialize the maximum representable value
 */
const double hub_float::maxVal = []() {
    double d;
    uint64_t maxBitsCopy = hub_float::maxBits;
    std::memcpy(&d, &maxBitsCopy, sizeof(d));
    return d;
}();

/**
 * @brief Initialize the minimum representable value
 */
const double hub_float::minVal = []() {
    double d;
    uint64_t minBitsCopy = hub_float::minBits;
    std::memcpy(&d, &minBitsCopy, sizeof(d));
    return d;
}();

const double hub_float::lowestVal = []() {
    double d;
    uint64_t minPosBitsCopy = hub_float::minPosBits;
    std::memcpy(&d, &minPosBitsCopy, sizeof(d));
    return d;
}();

// -------------------------------------------------------------------
// Implementation of hub_float member functions
// -------------------------------------------------------------------

/**
 * @brief Default constructor implementation
 */
hub_float::hub_float() : value(0.0) {}

/**
 * @brief Float constructor implementation
 * @param f The float value to convert
 */
hub_float::hub_float(float f) : hub_float(static_cast<double>(f)) {}


/**
 * @brief Double constructor implementation
 * @param d The double value to convert
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

/**
 * @brief Construct a hub_float from a raw binary representation
 * @param binary_value The raw binary value representing the hub_float (sign, exponent, mantissa)
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



/**
 * @brief Conversion to double implementation
 * @return The internal value as a double
 */
hub_float::operator double() const {
    return value;
}

/**
 * @brief Quantize a double to the hub_float grid
 * @param d The double value to quantize
 * @return The quantized double value
 */
double hub_float::quantize(double d)
{
    double special_result;
    return handle_special_cases(d, special_result) ? special_result : apply_hub_grid(d);
}

/**
 * @brief Handle special cases in floating-point operations
 * @param d The input double value
 * @param result The output result if a special case is detected
 * @return True if a special case was handled, false otherwise
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

/**
 * @brief Check if a double value is already on the hub grid
 * @param d The double value to check
 * @return True if the value is on the grid, false otherwise
 */
inline bool hub_float::is_on_grid(double d) {
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(d));
    return (bits & ((1ULL << SHIFT) - 1)) == HUB_BIT;
}

/**
 * @brief Apply the hub grid to a double value
 * @param d The double value to quantize
 * @return The quantized double value
 */
inline double hub_float::apply_hub_grid(double d) {
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(d));
    bits = (bits & ~((1ULL << (SHIFT-1)) - 1)) | HUB_BIT;
    std::memcpy(&d, &bits, sizeof(d));
    return d;
}


/**
 * @brief Handle special values (NaN, subnormal)
 * @param d The input double value
 * @return The handled result
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

/**
 * @brief Addition operator implementation
 * @param other The hub_float to add
 * @return A new hub_float containing the sum
 */
hub_float hub_float::operator+(const hub_float &other) const {
    return quantize(this->value + other.value);
}

/**
 * @brief Subtraction operator implementation
 * @param other The hub_float to subtract
 * @return A new hub_float containing the difference
 */
hub_float hub_float::operator-(const hub_float &other) const {
    return quantize(this->value - other.value);
}

/**
 * @brief Multiplication operator implementation
 * @param other The hub_float to multiply by
 * @return A new hub_float containing the product
 */
hub_float hub_float::operator*(const hub_float &other) const {
    return quantize(this->value * other.value);
}

/**
 * @brief Division operator implementation
 * @param other The hub_float to divide by
 * @return A new hub_float containing the quotient
 */
hub_float hub_float::operator/(const hub_float &other) const {
    return quantize(this->value / other.value);
}

/**
 * @brief Addition assignment operator implementation
 * @param other The hub_float to add
 * @return Reference to this object after addition
 */
hub_float& hub_float::operator+=(const hub_float &other) {
    *this = *this + other;
    return *this;
}

/**
 * @brief Subtraction assignment operator implementation
 * @param other The hub_float to subtract
 * @return Reference to this object after subtraction
 */
hub_float& hub_float::operator-=(const hub_float &other) {
    *this = *this - other;
    return *this;
}

/**
 * @brief Multiplication assignment operator implementation
 * @param other The hub_float to multiply by
 * @return Reference to this object after multiplication
 */
hub_float& hub_float::operator*=(const hub_float &other) {
    *this = *this * other;
    return *this;
}

/**
 * @brief Division assignment operator implementation
 * @param other The hub_float to divide by
 * @return Reference to this object after division
 */
hub_float& hub_float::operator/=(const hub_float &other) {
    *this = *this / other;
    return *this;
}

/**
 * @brief Extract the bit fields from the internal representation
 * @return A BitFields structure containing the extracted fields
 */
hub_float::BitFields hub_float::extractBitFields() const {
    hub_float::BitFields fields;
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    
    // Extract components
    fields.sign = (bits >> 63) & 1;
    int double_exp = static_cast<int>((bits >> 52) & 0x7FF);
    fields.fraction = bits & ((1ULL << 52) - 1ULL);

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

/**
 * @brief Convert the hub_float to a binary string representation
 * @return String in S|EEEEEEEE|MMMMMMMMMMMMMMMMMMMMMMMM format
 * @see extractBitFields()
 * @see toHexString()
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

/**
 * @brief Convert the hub_float to a hexadecimal string representation
 * 
 * This function converts the hub_float value to a hexadecimal string representation.
 * The resulting string includes the sign bit, exponent bits, and mantissa bits,
 * packed together and represented as a hexadecimal number.
 * 
 * @return A string containing the hexadecimal representation of the hub_float value,
 *         prefixed with "0x" and padded with leading zeros if necessary.
 */
std::string hub_float::toHexString() const {
    hub_float::BitFields fields = extractBitFields();
    
    // Pack components
    const int total_bits = 1 + EXP_BITS + MANT_BITS;
    const int hex_digits = (total_bits + 3) / 4; // Ceiling division
    const uint64_t packed = (static_cast<uint64_t>(fields.sign) << (EXP_BITS + MANT_BITS)) |
                           (static_cast<uint64_t>(fields.custom_exp) << MANT_BITS) |
                           fields.custom_frac;

    // Format hex string
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase 
        << std::setw(hex_digits) << std::setfill('0') << packed;
    
    return oss.str();
}


// -------------------------------------------------------------------
// Non-member functions
// -------------------------------------------------------------------

/**
 * @brief Stream insertion operator for hub_float
 * 
 * This operator allows hub_float objects to be output to standard output streams.
 * It outputs the internal double value of the hub_float object.
 * 
 * @param os The output stream to write to
 * @param hf The hub_float object to output
 * @return A reference to the output stream
 */
std::ostream& operator<<(std::ostream &os, const hub_float &hf) {
    os << hf.value;
    return os;
}

/**
 * @brief Calculate the square root of a hub_float value
 * 
 * This function computes the square root of the given hub_float value.
 * The result is quantized to ensure it conforms to the hub_float representation.
 * 
 * @param x The hub_float value to calculate the square root of
 * @return A new hub_float object representing the square root of x
 */
hub_float sqrt(const hub_float &x) {
    return hub_float::quantize(std::sqrt(static_cast<double>(x)));
}


/**
 * @brief User-defined literal for creating hub_float objects
 * 
 * This operator allows for the creation of hub_float objects using a literal suffix.
 * It converts a long double literal to a hub_float object.
 * 
 * @param d The long double value to convert to a hub_float
 * @return A new hub_float object representing the given value
 */
hub_float operator"" _hb(long double d) {
    return hub_float(static_cast<double>(d));
}

// -------------------------------------------------------------------
// End of hub_float.cpp
// -------------------------------------------------------------------

