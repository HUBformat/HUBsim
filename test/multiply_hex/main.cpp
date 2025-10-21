#include <iostream>
#include <string>
#include <cstdint>
#include <iomanip>
#include "hub_float.hpp"

int main() {
    std::cout << "=== HUB Float Multiplication Test ===" << std::endl;
    
    // Define the two hex values
    uint32_t hex1 = 0x461C9CF6;
    uint32_t hex2 = 0x63E8CAE0;  // Note: 861191c9a would be 9 hex digits (36 bits), using first 8
    
    // Create hub_float objects from the hex values
    hub_float a(hex1);
    hub_float b(hex2);
    
    // Display input values
    std::cout << "\nInput 1:" << std::endl;
    std::cout << "  Hex: " << a.toHexString() << std::endl;
    std::cout << "  Binary: " << a.toBinaryString() << std::endl;
    std::cout << "  Decimal: " << std::scientific << std::setprecision(17) 
              << static_cast<double>(a) << std::endl;
    
    std::cout << "\nInput 2:" << std::endl;
    std::cout << "  Hex: " << b.toHexString() << std::endl;
    std::cout << "  Binary: " << b.toBinaryString() << std::endl;
    std::cout << "  Decimal: " << std::scientific << std::setprecision(17) 
              << static_cast<double>(b) << std::endl;
    
    // Perform multiplication
    hub_float result = a / b;
    
    // Display result
    std::cout << "\nResult (a / b):" << std::endl;
    std::cout << "  Hex: " << result.toHexString() << std::endl;
    std::cout << "  Binary: " << result.toBinaryString() << std::endl;
    std::cout << "  Decimal: " << std::scientific << std::setprecision(17) 
              << static_cast<double>(result) << std::endl;
    
    return 0;
}
