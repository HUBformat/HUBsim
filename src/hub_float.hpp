/*
    File: hub_float.hpp
    Header file for the hub_float class, a custom floating-point implementation with configurable exponent and mantissa bits.

    This class implements a custom floating-point format with a "hub" bit.
*/

#ifndef HUB_FLOAT_HPP
#define HUB_FLOAT_HPP

#include <iostream>
#include <cstdint>  // For uint64_t



/*
    Constant: EXP_BITS
    Number of bits for the exponent field (default: 8 for single precision).
*/
#ifndef EXP_BITS
#define EXP_BITS 8 // Double: 11
#endif

/*
    Constant: MANT_BITS
    Number of bits for the mantissa field (default: 23 for single precision).
*/
#ifndef MANT_BITS
#define MANT_BITS 23 // Double: 52
#endif

/*
    Class: hub_float
    A custom floating-point class with configurable precision and a "hub" bit for consistent rounding.

    The hub_float class implements a floating-point format that uses a special "hub" bit.
    Internally, values are stored as doubles that are quantized to lie on a specific grid determined by the exponent,
    mantissa, and the extra "hub" bit (an implicit least significant bit).
*/
class hub_float {
public:
    /*
        Function: hub_float
        Default constructor, initializes to zero.
    */
    hub_float();              // Default constructor.

    /*
        Function: hub_float
        Construct from float.

        Parameters:
        f - The float value to convert.
    */
    hub_float(float f);       // Construct from float.

    /*
        Function: hub_float
        Construct from double.

        Parameters:
        d - The double value to convert.
    */
    hub_float(double d);      // Construct from double.

    /*
        Function: hub_float
        Construct from int.

        Parameters:
        i - The int value to convert.
    */
    hub_float(int i);         // Construct from int.

    /*
        Function: hub_float
        Construct a hub_float from a raw binary representation.

        Parameters:
        binary_value - The raw binary value representing the hub_float (sign, exponent, mantissa).
    */
    hub_float(uint32_t binary_value);
    
    /*
        Function: operator double
        Conversion operator to double.

        Returns:
        The internal value as a double.
    */
    operator double() const;
    
    /*
        Function: operator+
        Addition operator.

        Parameters:
        other - The hub_float to add.

        Returns:
        A new hub_float containing the sum.
    */
    hub_float operator+(const hub_float &other) const;

    /*
        Function: operator-
        Subtraction operator.

        Parameters:
        other - The hub_float to subtract.

        Returns:
        A new hub_float containing the difference.
    */
    hub_float operator-(const hub_float &other) const;
    

    /*
        Function: operator*
        Multiplication operator.

        Parameters:
        other - The hub_float to multiply by.

        Returns:
        A new hub_float containing the product.
    */
    hub_float operator*(const hub_float &other) const;
    
    /*
        Function: operator/
        Division operator.

        Parameters:
        other - The hub_float to divide by.

        Returns:
        A new hub_float containing the quotient.
    */
    hub_float operator/(const hub_float &other) const;
    
    /*
        Function: operator+=
        Addition assignment operator.

        Parameters:
        other - The hub_float to add.

        Returns:
        Reference to this object after addition.
    */    
    hub_float& operator+=(const hub_float &other);

    /*
        Function: operator-=
        Subtraction assignment operator.

        Parameters:
        other - The hub_float to subtract.

        Returns:
        Reference to this object after subtraction.
    */   
    hub_float& operator-=(const hub_float &other);

    /*
        Function: operator*=
        Multiplication assignment operator.

        Parameters:
        other - The hub_float to multiply by.

        Returns:
        Reference to this object after multiplication.
    */   
    hub_float& operator*=(const hub_float &other);

    /*
        Function: operator/=
        Division assignment operator.

        Parameters:
        other - The hub_float to divide by.

        Returns:
        Reference to this object after division.
    */   
    hub_float& operator/=(const hub_float &other);

    /*
       Struct: BitFields
       Structure to hold the extracted bit fields of a hub_float.
       
       Fields:
       sign - Sign of the number (0 or 1).
       custom_exp - Custom exponent value.
       fraction - Fractional part of the number.
       custom_frac - Custom fractional part without the "hub" bit.
       custom_frac_with_hub - Custom fractional part including the "hub" bit.
   */
    struct BitFields {
        int sign;
        int custom_exp;
        uint64_t fraction;
        uint64_t custom_frac;
        uint64_t custom_frac_with_hub;
    };

   /*
       Function: extractBitFields
       Extract the bit fields from the internal representation.

       Returns:
       A BitFields structure containing the extracted fields.
   */
    BitFields extractBitFields() const;

   /*
       Function: toBinaryString
       Convert the hub_float to a binary string representation.

       Returns:
       A string in the format "S|EEEEEEEE|MMMMMMMMMMMMMMMMMMMMMMMM".
   */
    std::string toBinaryString() const;
   
   /*
       Function: toHexString
       Convert the hub_float to a hexadecimal string representation.

       Returns:
       A string in the format "0xXXXXXXXX".
   */
    std::string toHexString() const;

   /*
       Friend Function: sqrt
       Square root function for hub_float.

       Parameters:
       x - The hub_float to compute the square root of.

       Returns:
       The square root as a hub_float.
   */
    friend hub_float sqrt(const hub_float& x);

   /*
       Friend Function: operator<<
       Stream insertion operator for hub_float.

       Parameters:
       os - The output stream.
       hf - The hub_float to output.

       Returns:
       Reference to the output stream.
   */
    friend std::ostream& operator<<(std::ostream &os, const hub_float &hf);

   /*
      Constant: lowestVal
      Lowest representable negative value in hub_float format as a double.
   */
    static const double lowestVal;

private:
    /*
        Variable: value
        Internal value stored as a double that lies on the custom grid.
    */
    double value;

    /*
        Function: float_to_hub
        Force the extra (24th) significand bit in a double converted from a normalized float.

        Parameters:
        d - The double value to convert.

        Returns:
        The converted double with the hub bit set.
    */
    static double float_to_hub(double d);

    /*
        Function: quantize
        Quantize a double result to the hub_float grid.

        Parameters:
        d - The double value to quantize.

        Returns:
        The quantized double value.
    */
    static double quantize(double d);
    
    /*
        Function: handle_special_cases
        Handle special cases in floating-point operations, such as NaN or infinities.

        Parameters:
        d - The input double value.
        result - The output result if a special case is detected.

        Returns:
        True if a special case was handled, false otherwise.
    */
    static bool handle_special_cases(double d, double& result);

    /*
        Function: handle_specials
        Handle special values like NaN or subnormal numbers.

        Parameters:
        d - The input double value.

        Returns:
        The processed result for special values.
    */
    static double handle_specials(double d);

    /*
        Function: is_on_grid
        Check if a double value is already on the hub grid.

        Parameters:
        d - The double value to check.

        Returns:
        True if the value is on the grid, false otherwise.
    */   
    static bool is_on_grid(double d);

    /*
        Function: apply_hub_grid
        Apply the hub grid to a double value.

        Parameters:
        d - The double value to quantize.

        Returns:
        The quantized double value.
    */   
    static double apply_hub_grid(double d);

    /*
       Constant: SHIFT
       Number of low-order bits in the double's mantissa that will be forced or cleared.
    */
    static constexpr int SHIFT = 52 - MANT_BITS;

    /*
       Constant: HUB_BIT
       The bit used to emulate the "implicit leading 1" in normalized IEEE format.
    */
    static const uint64_t HUB_BIT = (1ULL << (SHIFT - 1));

    /*
       Constant: CUSTOM_BIAS
       The bias for the custom exponent format. This can be configured based on IEEE or custom rules.
    */
    #ifdef ORIGINAL_IEE_BIAS
    static const int CUSTOM_BIAS = (1 << (EXP_BITS - 1)) - 1;
    #else
    static const int CUSTOM_BIAS = (1 << (EXP_BITS - 1));
    #endif

    /*
       Constant: BIAS_DIFF
       The difference between IEEE double bias (1023) and custom bias.
    */
    static const int BIAS_DIFF = 1023 - CUSTOM_BIAS; 
    
    /*
       Constant: CUSTOM_MAX_EXP
       Maximum value for the custom exponent field.
    */
    static constexpr int CUSTOM_MAX_EXP = (1 << EXP_BITS) - 1;

    /*
       Constant: doubleExp
       Maximum value for the IEEE double exponent corresponding to the custom maximum exponent.
    */
    static constexpr int doubleExp = CUSTOM_MAX_EXP + BIAS_DIFF;

    /*
       Constant: customFrac
       Maximum custom significand with all bits set, excluding the "hub" bit.
    */
    static constexpr uint64_t customFrac = ((1ULL << (MANT_BITS + 1)) - 1) & ~(1ULL << 1);
    
    /*
       Constant: doubleFrac
       Double fraction field corresponding to maximum custom fraction.
    */
    static constexpr uint64_t doubleFrac = customFrac << (SHIFT - 1);

    /*
       Constant: maxBits
       Bit pattern for the maximum positive representable value in hub_float format.
    */
    static constexpr uint64_t maxBits = (static_cast<uint64_t>(doubleExp) << 52) | doubleFrac;

    /*
       Constant: minBits
       Bit pattern for the minimum negative representable value in hub_float format.
    */    
    static constexpr uint64_t minBits = (1ULL << 63) | (static_cast<uint64_t>(doubleExp) << 52) | doubleFrac;

    /*
       Constant: customMinFrac
       Minimum custom significand with only the least significant bit set to 1.
    */
    static constexpr uint64_t customMinFrac = 1ULL; 


    /*
       Constant: doubleMinFrac
       Double fraction field corresponding to minimum custom fraction.
    */
    static constexpr uint64_t doubleMinFrac = customMinFrac << (SHIFT - 1);
    
    /*
       Constant: minPosBits
       Bit pattern for the minimum positive representable value in hub_float format.
    */
    static constexpr uint64_t minPosBits = (static_cast<uint64_t>(BIAS_DIFF) << 52) | doubleMinFrac;


    /*
       Constant: maxVal
       Maximum representable value in hub_float format as a double.
    */
    static const double maxVal;

    /*
       Constant: minVal
       Minimum representable positive value in hub_float format as a double.
    */    
    static const double minVal;

};

/*
    Function: operator"" _hb
    User-defined literal for creating a hub_float.

    Parameters:
    d - The long double value to convert.

    Returns:
    A hub_float representation of the provided value.
*/
hub_float operator"" _hb(long double d);

#endif // HUB_FLOAT_HPP
