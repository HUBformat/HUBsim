#include <iostream>
#include <string>
#include <cstdint>
#include <iomanip>
#include "hub_float.hpp"

uint32_t binaryStringToUint32(const std::string& binary_str) {
    uint32_t result = 0;
    for (size_t i = 0; i < binary_str.length() && i < 32; ++i) {
        if (binary_str[i] == '1') {
            result |= (1U << (binary_str.length() - 1 - i));
        } else if (binary_str[i] != '0') {
            throw std::invalid_argument("Invalid binary string - contains non-binary characters");
        }
    }
    return result;
}

void printHubFloatInfo(const std::string& binary_input) {
    try {
        // Convert binary string to uint32_t
        uint32_t binary_value = binaryStringToUint32(binary_input);
        
        // Create hub_float from binary representation
        hub_float hf(binary_value);
        
        // Print detailed information
        std::cout << "\n=== Hub Float Analysis ===" << std::endl;
        std::cout << "Binary input: " << binary_input << std::endl;
        std::cout << "Hex value: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << binary_value << std::dec << std::endl;
        
        // Extract bit fields for analysis
        auto fields = hf.extractBitFields();
        std::cout << "Sign bit: " << fields.sign << std::endl;
        std::cout << "Custom exponent: " << fields.custom_exp << " (0x" << std::hex << fields.custom_exp << std::dec << ")" << std::endl;
        std::cout << "Custom fraction: " << fields.custom_frac << " (0x" << std::hex << fields.custom_frac << std::dec << ")" << std::endl;
        
        std::cout << "\n=== Decimal Values ===" << std::endl;
        std::cout << "Decimal (full precision): " << std::fixed << std::setprecision(17) << static_cast<double>(hf) << std::endl;
        std::cout << "Decimal (scientific): " << std::scientific << std::setprecision(17) << static_cast<double>(hf) << std::endl;
        std::cout << "Decimal (default format): " << std::defaultfloat << static_cast<double>(hf) << std::endl;
        
        std::cout << "\n=== Hub Float Representations ===" << std::endl;
        std::cout << "Hub_float binary: " << hf.toBinaryString() << std::endl;
        std::cout << "Hub_float hex: " << hf.toHexString() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        // Use command line argument
        std::string binary_input = argv[1];
        printHubFloatInfo(binary_input);
    } else {
        // Use the default value from the request
        std::string binary_input = "00111111100000000000000000000000";
        std::cout << "Using default binary input: " << binary_input << std::endl;
        printHubFloatInfo(binary_input);
        
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "You can also run this program with a custom binary string:" << std::endl;
        std::cout << "./binary_to_decimal_interactive <32-bit-binary-string>" << std::endl;
    }
    
    return 0;
}
