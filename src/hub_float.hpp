#ifndef HUB_FLOAT_HPP
#define HUB_FLOAT_HPP

#include <iostream>

#ifndef EXP_BITS
#define EXP_BITS 8 // Double: 11
#endif

#ifndef MANT_BITS
#define MANT_BITS 23 // Double: 52
#endif

class hub_float {
public:
    // Constructors
    hub_float();              // Default constructor.
    hub_float(float f);       // Construct from float.
    hub_float(double d);      // Construct from double.
    
    // Conversion operator to double.
    operator double() const;
    
    // Arithmetic operators.
    hub_float operator+(const hub_float &other) const;
    hub_float operator-(const hub_float &other) const;
    hub_float operator*(const hub_float &other) const;
    hub_float operator/(const hub_float &other) const;
    
    hub_float& operator+=(const hub_float &other);
    hub_float& operator-=(const hub_float &other);
    hub_float& operator*=(const hub_float &other);
    hub_float& operator/=(const hub_float &other);

    std::string toBinaryString() const;
    std::string toHexString32() const;


    friend hub_float sqrt(const hub_float& x);

    // Friend function to output the hub_float.
    friend std::ostream& operator<<(std::ostream &os, const hub_float &hf);

private:
    // Internally, the value is stored as a double that lies on our custom grid.
    double value;

    // Helper: Force the extra (24th) significand bit in a double that was obtained
    // by an exact conversion from a normalized float.
    static double float_to_hub(double d);

    // Helper: Quantize a double result (using downward rounding).
    static hub_float quantize(double d);
    static double handle_specials(double d);

    
    // SHIFT = number of low-order bits in the double's mantissa that we will force/clear.
    static constexpr int SHIFT = 52 - MANT_BITS;

    // The bit (within bits 0..51 of a double's mantissa) we use to emulate the
    // "implicit leading 1" in a normalized IEEE style. For single precision (23 mantissa),
    // SHIFT = 29, so HUB_BIT = 1 << 28, matching the old code.
    static const uint64_t HUB_BIT = (1ULL << (SHIFT - 1));

    // The difference in biases: double's bias = 1023, custom bias = (1<<(EXP_BITS-1)) - 1.
    static const int CUSTOM_BIAS = (1 << (EXP_BITS - 1)) - 1;
    static const int BIAS_DIFF = 1023 - CUSTOM_BIAS; 
    // For single precision (EXP_BITS=8, MANT_BITS=23), BIAS_DIFF = 1023 - 127 = 896.

    static constexpr int CUSTOM_MAX_EXP = (1 << EXP_BITS) - 2; // 254
    static constexpr int doubleExp = CUSTOM_MAX_EXP + BIAS_DIFF; // 254 + 896 = 1150
    // Maximum custom significand (MANT_BITS+1 bits all ones)
    static constexpr uint64_t customFrac = (1ULL << (MANT_BITS + 1)) - 1; // (1<<24)-1 = 0xFFFFFF
    // The double's fraction field is our custom fraction shifted left by (SHIFT-1) bits.
    static constexpr uint64_t doubleFrac = customFrac << (SHIFT - 1);

    // Build the 64-bit bit patterns for the max number (including ILSB)
    static constexpr uint64_t maxBits = (static_cast<uint64_t>(doubleExp) << 52) | doubleFrac;
    static constexpr uint64_t minBits = (1ULL << 63) | (static_cast<uint64_t>(doubleExp) << 52) | doubleFrac;

    static const double maxVal;
    static const double minVal;
};

// User–defined literal for hub_float (literal suffix must start with an underscore).
hub_float operator"" _hb(long double d);

#endif // HUB_FLOAT_HPP
