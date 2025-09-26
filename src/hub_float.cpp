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
    Constructor that converts an int to a hub_float.

    Parameters:
        i - The int value to convert.
*/
hub_float::hub_float(int i) : hub_float(static_cast<double>(i)) {}

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

    #if UNBIASED_ROUNDING
        // Check if all the bits we are truncating are zeros
        bool all_truncated_bits_zero = ((bits & ((1ULL << (SHIFT-1)) - 1)) == 0);
        
        if (all_truncated_bits_zero) {
            // std::cout << "Unbiased rounding enabled" << std::endl;
            uint64_t clear_mask = ~(1ULL << SHIFT);
            bits = (bits & clear_mask) | HUB_BIT;
        } else {
            // Standard behavior - set HUB_BIT and clear all lower bits
            bits = (bits & ~((1ULL << (SHIFT-1)) - 1)) | HUB_BIT;
        }
    #else
        // Standard behavior - set HUB_BIT and clear all lower bits
        bits = (bits & ~((1ULL << (SHIFT-1)) - 1)) | HUB_BIT;
    #endif

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

    // Mask to only keep the bits we need to avoid sign extension issues
    const uint64_t mask = (1ULL << total_bits) - 1;
    const uint64_t masked_packed = packed & mask;

    // Format hex string
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase 
        << std::setw(hex_digits) << std::setfill('0') << masked_packed;
    
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
   Function: fma
   Fused multiply-add function for hub_float.
   Computes (a*b + c) with only one rounding operation using hardware FMA.

   Parameters:
       a - The first hub_float to multiply.
       b - The second hub_float to multiply.
       c - The hub_float to add to the product.

   Returns:
       The result of (a*b + c) as a hub_float.

   Notes:
       When emulating float32 (EXP_BITS=8, MANT_BITS=23), hardware FMA operates in double precision,
       which can lead to double rounding.
       This function includes special logic to detect and correct such cases by adjusting the result
       by one ULP when necessary, ensuring accurate single-precision rounding.
*/
hub_float fma(const hub_float& a, const hub_float& b, const hub_float& c) {
    // Extract the underlying double-precision values from the hub_float objects.
    double val_a = static_cast<double>(a);
    double val_b = static_cast<double>(b);
    double val_c = static_cast<double>(c);

    // Raw fma
    double sumDouble = std::fma(val_a, val_b, val_c);

    // Special rounding logic for avoiding double rounding when emulating float32
    #if (EXP_BITS == 8) && (MANT_BITS == 23)
        // Check if all bits after HUB_BIT are 0
        uint64_t sum_bits;
        std::memcpy(&sum_bits, &sumDouble, sizeof(sum_bits));

        // Extract the double's mantissa (lower 52 bits)
        uint64_t mantissa = sum_bits & ((1ULL << 52) - 1);

        // Create mask for all bits after HUB_BIT
        uint64_t mask_relevant_bits = ((1ULL << (hub_float::SHIFT - 1)) - 1);

        // Check if all bits after HUB_BIT are zero
        bool relevant_bits_after_hub_zero = (mantissa & mask_relevant_bits) == 0;

        // Need to substract 1 ULP if true
        bool needs_correction = false;

        if (relevant_bits_after_hub_zero) {
            // Compute product separately and analyze fields
            double intermediate_product = val_a * val_b;
            hub_float temp_product(intermediate_product);
            auto c_fields = c.extractBitFields();
            auto product_fields = temp_product.extractBitFields();

            if (c_fields.custom_exp > product_fields.custom_exp) {
                // Case where sum is greater than product (product should shift to exponent of sum)
                uint64_t prod_bits;
                std::memcpy(&prod_bits, &intermediate_product, sizeof(prod_bits));
                uint64_t prod_mantissa = prod_bits & ((1ULL << 52) - 1);
                bool bit_24_set = (prod_mantissa & (1ULL << 24)) != 0;
                if (bit_24_set) {
                    needs_correction = true;
                }
            } else if (c_fields.custom_exp < product_fields.custom_exp) {
                int shift_amount = product_fields.custom_exp - c_fields.custom_exp;
                uint64_t c_bits;
                std::memcpy(&c_bits, &val_c, sizeof(c_bits));
                uint64_t c_mantissa = c_bits & ((1ULL << 52) - 1);

                bool c_contributes_to_last_3_bits = false;
                if (shift_amount <= 52) {
                    // Mask for LSBs that will be shifted out
                    uint64_t mask_lost_bits = (1ULL << shift_amount) - 1;

                    if ((c_mantissa & mask_lost_bits) != 0) {
                        c_contributes_to_last_3_bits = true;
                    }
                } else {
                    // Entire mantissa shifted out
                    if (c_mantissa != 0) {
                        c_contributes_to_last_3_bits = true;
                    }
                }

                if (c_contributes_to_last_3_bits) {
                    needs_correction = true;
                }
            }
        }

        // If correction is needed, subtract one ULP
        if (needs_correction) {
            uint64_t sbits;
            std::memcpy(&sbits, &sumDouble, sizeof(sbits));
            uint64_t expBits = (sbits >> 52) & 0x7FF;
            double ulp_val;
            uint64_t ulpBits = (expBits << 52) | (1ULL << 29);
            std::memcpy(&ulp_val, &ulpBits, sizeof(ulp_val));

            sumDouble -= ulp_val;
        }
    #endif // (EXP_BITS == 8) && (MANT_BITS == 23)

    hub_float result = hub_float::quantize(sumDouble);
    return result;
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

