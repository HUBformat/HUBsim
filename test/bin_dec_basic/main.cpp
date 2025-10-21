#include <iostream>
#include <string>
#include <cstdint>
#include <iomanip>
#include "hub_float.hpp"

int main() {
    // Test 1: The binary string input conversion
    std::cout << "=== Test 1: Binary to Decimal Conversion ===" << std::endl;
    std::string binary_input = "00000000000000000000000000000001";
    
    // Convert binary string to uint32_t
    uint32_t binary_value = 0;
    for (size_t i = 0; i < binary_input.length(); ++i) {
        if (binary_input[i] == '1') {
            binary_value |= (1U << (binary_input.length() - 1 - i));
        }
    }
    
    // Create hub_float from binary representation
    hub_float hf(binary_value);
    
    // Print the results
    std::cout << "Binary input: " << binary_input << std::endl;
    std::cout << "Hex value: 0x" << std::hex << std::uppercase << binary_value << std::dec << std::endl;
    std::cout << "Decimal value (full precision): " << std::fixed << std::setprecision(17) << static_cast<double>(hf) << std::endl;
    std::cout << "Decimal value (scientific): " << std::scientific << std::setprecision(17) << static_cast<double>(hf) << std::endl;
    
    // Also show the hub_float's binary representation to verify
    std::cout << "Hub_float binary representation: " << hf.toBinaryString() << std::endl;
    std::cout << "Hub_float hex representation: " << hf.toHexString() << std::endl;
    
    // Test 2: Verify that decimal value 2.93873640254264289E-39 converts to 0x00000001
    std::cout << "\n=== Test 2: Decimal to Hub_float Conversion Test ===" << std::endl;
    double test_value = 2.93873640254264289E-39;
    hub_float a(test_value);
    
    std::cout << "Input decimal value: " << std::scientific << std::setprecision(17) << test_value << std::endl;
    std::cout << "Hub_float hex representation: " << a.toHexString() << std::endl;
    std::cout << "Hub_float binary representation: " << a.toBinaryString() << std::endl;
    std::cout << "Hub_float decimal value: " << std::scientific << std::setprecision(17) << static_cast<double>(a) << std::endl;
    
    // Check if the hex representation is 0x00000001
    std::string expected_hex = "0x00000001";
    std::string actual_hex = a.toHexString();
    
    std::cout << "\n=== Verification ===" << std::endl;
    std::cout << "Expected hex: " << expected_hex << std::endl;
    std::cout << "Actual hex:   " << actual_hex << std::endl;
    std::cout << "Match: " << (actual_hex == expected_hex ? "YES" : "NO") << std::endl;
    
    // Also compare with the original binary conversion
    std::cout << "\n=== Comparison with Binary Conversion ===" << std::endl;
    std::cout << "Binary->hub_float decimal: " << std::scientific << std::setprecision(17) << static_cast<double>(hf) << std::endl;
    std::cout << "Decimal->hub_float decimal: " << std::scientific << std::setprecision(17) << static_cast<double>(a) << std::endl;
    std::cout << "Values equal: " << (static_cast<double>(hf) == static_cast<double>(a) ? "YES" : "NO") << std::endl;
    
    return 0;
}
