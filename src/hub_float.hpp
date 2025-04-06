/**
 * @file hub_float.hpp
 * @brief Header file for the hub_float class, a custom floating-point implementation with configurable exponent and mantissa bits.
 * @details This class implements a custom floating-point format with a "hub" bit.
 */

#ifndef HUB_FLOAT_HPP
#define HUB_FLOAT_HPP

#include <iostream>
#include <cstdint>  // For uint64_t


/**
 * @brief Number of bits for the exponent field (default: 8 for single precision)
 */
#ifndef EXP_BITS
#define EXP_BITS 8 // Double: 11
#endif

/**
 * @brief Number of bits for the mantissa field (default: 23 for single precision)
 */
#ifndef MANT_BITS
#define MANT_BITS 23 // Double: 52
#endif

/**
 * @class hub_float
 * @brief A custom floating-point class with configurable precision and a "hub" bit for consistent rounding
 * 
 * The hub_float class implements a floating-point format that uses a special "hub" bit.
 * Internally, values are stored as doubles that are quantized to lie on a specific
 * grid determined by the exponent and mantissa and the extra "hub" bit, an implicit least significant bit.
 */
class hub_float {
public:
    /**
     * @brief Default constructor, initializes to zero
     */
    hub_float();              // Default constructor.

    /**
     * @brief Construct from float
     * @param f The float value to convert
     */
    hub_float(float f);       // Construct from float.

    /**
     * @brief Construct from double
     * @param d The double value to convert
     */
    hub_float(double d);      // Construct from double.

    
    /**
     * @brief Construct from int (to resolve ambiguity with literals)
     * @param i The integer value to convert
     */
    hub_float(int i);         // Construct from int.
    
    /**
    * @brief Construct a hub_float from a raw binary representation
    * @param binary_value The raw binary value representing the hub_float (sign, exponent, mantissa)
    */
    hub_float(uint32_t binary_value);
    
    /**
     * @brief Conversion operator to double
     * @return The internal value as a double
     */
    operator double() const;
    
    /**
     * @brief Addition operator
     * @param other The hub_float to add
     * @return A new hub_float containing the sum
     */
    hub_float operator+(const hub_float &other) const;

    /**
     * @brief Subtraction operator
     * @param other The hub_float to subtract
     * @return A new hub_float containing the difference
     */    
    hub_float operator-(const hub_float &other) const;
    
    /**
     * @brief Multiplication operator
     * @param other The hub_float to multiply by
     * @return A new hub_float containing the product
     */    
    hub_float operator*(const hub_float &other) const;
    
    /**
     * @brief Division operator
     * @param other The hub_float to divide by
     * @return A new hub_float containing the quotient
     */
    hub_float operator/(const hub_float &other) const;
    
    /**
     * @brief Addition assignment operator
     * @param other The hub_float to add
     * @return Reference to this object after addition
     */    
    hub_float& operator+=(const hub_float &other);

    /**
     * @brief Subtraction assignment operator
     * @param other The hub_float to subtract
     * @return Reference to this object after subtraction
     */    
    hub_float& operator-=(const hub_float &other);

    /**
     * @brief Multiplication assignment operator
     * @param other The hub_float to multiply by
     * @return Reference to this object after multiplication
     */    
    hub_float& operator*=(const hub_float &other);

    /**
     * @brief Division assignment operator
     * @param other The hub_float to divide by
     * @return Reference to this object after division
     */    
    hub_float& operator/=(const hub_float &other);

    /**
     * @struct BitFields
     * @brief Structure to hold the extracted bit fields of a hub_float
     */
    struct BitFields {
        int sign;
        int custom_exp;
        uint64_t fraction;
        uint64_t custom_frac;
        uint64_t custom_frac_with_hub;
    };

    /**
     * @brief Extract the bit fields from the internal representation
     * @return A BitFields structure containing the extracted fields
     */
    BitFields extractBitFields() const;

    /**
     * @brief Convert the hub_float to a binary string representation
     * @return A string in the format "S|EEEEEEEE|MMMMMMMMMMMMMMMMMMMMMMMM"
     */
    std::string toBinaryString() const;
    /**
     * @brief Convert the hub_float to a hexadecimal string representation
     * @return A string in the format "0xXXXXXXXX"
     */
    std::string toHexString() const;

    /**
     * @brief Square root function for hub_float
     * @param x The hub_float to compute the square root of
     * @return The square root as a hub_float
     */
    friend hub_float sqrt(const hub_float& x);

    /**
     * @brief Stream insertion operator for hub_float
     * @param os The output stream
     * @param hf The hub_float to output
     * @return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream &os, const hub_float &hf);


private:
    /**
     * @brief Internal value stored as a double that lies on the custom grid
     */
    double value;

    /**
     * @brief Force the extra (24th) significand bit in a double converted from a normalized float
     * @param d The double value to convert
     * @return The converted double with the hub bit set
     */
    static double float_to_hub(double d);

    /**
     * @brief Quantize a double result to the hub_float grid
     * @param d The double value to quantize
     * @return The quantized double value
     */
    static double quantize(double d);
    
    /**
     * @brief Handle special cases in floating-point operations
     * @param d The input double value
     * @param result The output result if a special case is detected
     * @return True if a special case was handled, false otherwise
     */
    static bool handle_special_cases(double d, double& result);
    /**
     * @brief Handle special values (NaN, subnormal)
     * @param d The input double value
     * @return The handled result
     */
    static double handle_specials(double d);
    /**
     * @brief Check if a double value is already on the hub grid
     * @param d The double value to check
     * @return True if the value is on the grid, false otherwise
     */    
    static bool is_on_grid(double d);
    /**
     * @brief Apply the hub grid to a double value
     * @param d The double value to quantize
     * @return The quantized double value
     */    
    static double apply_hub_grid(double d);

    /**
     * @brief Number of low-order bits in the double's mantissa that will be forced/cleared
     */
    static constexpr int SHIFT = 52 - MANT_BITS;

    /**
     * @brief The bit used to emulate the "implicit leading 1" in normalized IEEE format
     */
    static const uint64_t HUB_BIT = (1ULL << (SHIFT - 1));

    /**
     * @brief The bias for the custom exponent format
     */
    #ifdef ORIGINAL_IEE_BIAS
    static const int CUSTOM_BIAS = (1 << (EXP_BITS - 1)) - 1;
    #else
    static const int CUSTOM_BIAS = (1 << (EXP_BITS - 1));
    #endif

    /**
     * @brief The difference between IEEE double bias (1023) and custom bias
     */
    static const int BIAS_DIFF = 1023 - CUSTOM_BIAS; 
    
    /**
     * @brief Maximum value for the custom exponent
     */
    static constexpr int CUSTOM_MAX_EXP = (1 << EXP_BITS) - 1;

    /**
     * @brief Maximum value for the double exponent corresponding to custom max
     */
    static constexpr int doubleExp = CUSTOM_MAX_EXP + BIAS_DIFF;

    /**
     * @brief Maximum custom significand (all bits set)
     */    
    static constexpr uint64_t customFrac = ((1ULL << (MANT_BITS + 1)) - 1) & ~(1ULL << 1);
    
    /**
     * @brief Double fraction field corresponding to maximum custom fraction
     */
    static constexpr uint64_t doubleFrac = customFrac << (SHIFT - 1);

    /**
     * @brief Bit pattern for the maximum positive value
     */
    static constexpr uint64_t maxBits = (static_cast<uint64_t>(doubleExp) << 52) | doubleFrac;

    /**
     * @brief Bit pattern for the minimum negative value
     */    
    static constexpr uint64_t minBits = (1ULL << 63) | (static_cast<uint64_t>(doubleExp) << 52) | doubleFrac;
    /**
     * @brief Minimum custom fraction
     */ 
    static constexpr uint64_t customMinFrac = 1ULL; 

    /**
    * @brief Double fraction field corresponding to minimum custom fraction
    */
    static constexpr uint64_t doubleMinFrac = customMinFrac << (SHIFT - 1);
    
    /**
     * @brief Bit pattern for the minimum positive value
     */
    static constexpr uint64_t minPosBits = (static_cast<uint64_t>(BIAS_DIFF) << 52) | doubleMinFrac;


    /**
     * @brief Maximum representable value
     */
    static const double maxVal;

    /**
     * @brief Minimum representable value
     */    
    static const double minVal;

    static const double lowestVal;

};

/**
 * @brief User-defined literal for hub_float
 * @param d The long double value to convert
 * @return A hub_float representation of the value
 */
hub_float operator"" _hb(long double d);

#endif // HUB_FLOAT_HPP
