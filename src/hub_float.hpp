#ifndef HUB_FLOAT_HPP
#define HUB_FLOAT_HPP

#include <iostream>

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
};

// User–defined literal for hub_float (literal suffix must start with an underscore).
hub_float operator"" _hb(long double d);

#endif // HUB_FLOAT_HPP
