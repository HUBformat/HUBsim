#include "hub_float.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

int main() {
    
    std::cout << std::fixed << std::setprecision(50);

    hub_float a(2.49189384f);
    hub_float b(1.23456789f);

    hub_float sum = a + b;
    hub_float diff = a - b;
    hub_float prod = a * b;
    hub_float quot = a / b;

    std::cout << "a = " << static_cast<double>(a) << "\n";
    std::cout << "b = " << static_cast<double>(b) << "\n";
    std::cout << "a + b = " << static_cast<double>(sum) << "\n";
    std::cout << "a - b = " << static_cast<double>(diff) << "\n";
    std::cout << "a * b = " << static_cast<double>(prod) << "\n";
    std::cout << "a / b = " << static_cast<double>(quot) << "\n";

    hub_float sqrt_a = sqrt(a);
    hub_float sqrt_b = sqrt(b);
    std::cout << "sqrt(a) = " << static_cast<double>(sqrt_a) << "\n";
    std::cout << "sqrt(b) = " << static_cast<double>(sqrt_b) << "\n";

    // Using the user-defined literal.
    hub_float pi = 3.14159_hb;
    std::cout << "pi = " << static_cast<double>(pi) << "\n";
    std::cout << "sqrt(pi) = " << static_cast<double>(sqrt(pi)) << "\n";
    std::cout << "Binary representation of pi: " << sum.toBinaryString() << "\n";


    return 0;
}
