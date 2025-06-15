#ifndef TEST_CONFIG_HPP
#define TEST_CONFIG_HPP

#include <cstdint>

// Configuration constants
struct TestConfig {
    static constexpr uint64_t MAX_EXHAUSTIVE_TESTS = 500000;
    //static constexpr uint64_t MAX_EXHAUSTIVE_TESTS = -1; // Always enable exhaustive
    static constexpr uint64_t RANDOM_SAMPLE_SIZE = 100000;
    static constexpr unsigned int RANDOM_SEED = 42;
    static constexpr bool SHOW_DETAILED_OUTPUT = false;
    // Set to true to generate an additional CSV file with numeric values
    static constexpr bool OUTPUT_SEPARATE_NUMERIC_FILE = true; 
};

#endif // TEST_CONFIG_HPP
