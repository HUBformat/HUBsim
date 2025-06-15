#include "operation_tester.hpp"
#include "test_config.hpp"
#include "utils.hpp"
#include <functional>
#include <thread>
#include <limits> // Required for numeric_limits
#include <iomanip> // Required for setprecision
#include <optional> // Required for optional ofstream

OperationTester::OperationTester(std::string opName) 
    : rng_(TestConfig::RANDOM_SEED), opName_(std::move(opName)) {}

const std::string& OperationTester::getName() const {
    return opName_;
}

std::vector<std::pair<hub_float, std::string>> OperationTester::getSpecialValues() const {
    return {
        {hub_float(0.0), "Zero"},
        {hub_float(-0.0), "Negative Zero"},
        {hub_float(1.0), "One"},
        {hub_float(-1.0), "Negative One"},
        {hub_float(std::numeric_limits<double>::infinity()), "Infinity"},
        {hub_float(-std::numeric_limits<double>::infinity()), "Negative Infinity"},
        {hub_float(hub_float::lowestVal), "Min Positive"},
        {hub_float(-hub_float::lowestVal), "Min Negative"}
    };
}

// Implementation of OperationTesterImpl and createTester
template<typename Operation, OpType Type>
OperationTesterImpl<Operation, Type>::OperationTesterImpl(const std::string& opName, Operation operation)
    : OperationTester(opName), operation_(operation) {}

template<typename Operation, OpType Type>
void OperationTesterImpl<Operation, Type>::runTests() {
    performTesting();
}

template<typename Operation, OpType Type>
void OperationTesterImpl<Operation, Type>::runSpecialCaseTests() {
    // --- File Setup ---
    std::string hex_filename = Utils::generateFilename(opName_, false, true, false); // Hex file is default
    std::ofstream outfile_hex = Utils::openOutputFile(hex_filename);
    outfile_hex << std::setprecision(std::numeric_limits<long double>::max_digits10); // Precision needed for potential numeric output later

    std::optional<std::ofstream> outfile_num;
    std::string num_filename;
    if (TestConfig::OUTPUT_SEPARATE_NUMERIC_FILE) {
        num_filename = Utils::generateFilename(opName_, false, true, true); // Get numeric filename
        outfile_num.emplace(Utils::openOutputFile(num_filename)); // Create and open the numeric file stream
        *outfile_num << std::setprecision(std::numeric_limits<long double>::max_digits10);
    }

    // --- Header Writing ---
    // Hex Header
    if constexpr (Type == OpType::TERNARY) outfile_hex << "X,Y,Z,R,Description\n";
    else if constexpr (Type == OpType::BINARY) outfile_hex << "X,Y,Z,Description\n";
    else outfile_hex << "X,Z,Description\n";
    
    // Numeric Header (Optional)
    if (outfile_num) {
        if constexpr (Type == OpType::TERNARY) *outfile_num << "X_num,Y_num,Z_num,R_num,Description\n";
        else if constexpr (Type == OpType::BINARY) *outfile_num << "X_num,Y_num,Z_num,Description\n";
        else *outfile_num << "X_num,Z_num,Description\n";
    }
    
    auto specialValues = getSpecialValues();

    Utils::clearScreen();
    std::cout << "=== Testing " << opName_ << " Special Cases ===\n";

    // --- Data Writing ---
    for (size_t i = 0; i < specialValues.size(); ++i) {
        const auto& x = specialValues[i];
        if constexpr (Type == OpType::TERNARY) {
            for (const auto& y : specialValues) {
                for (const auto& z : specialValues) {
                    hub_float result = operation_(x.first, y.first, z.first);
                    std::string desc = x.second + " " + opName_ + " " + y.second + " " + z.second;
                    // Write Hex values
                    outfile_hex << x.first.toHexString().substr(2) << ","
                                << y.first.toHexString().substr(2) << ","
                                << z.first.toHexString().substr(2) << ","
                                << result.toHexString().substr(2) << ","
                                << desc << "\n";
                    // Conditionally write Numeric values
                    if (outfile_num) {
                        *outfile_num << x.first << "," << y.first << "," << z.first << "," << result << "," << desc << "\n";
                    }
                }
            }
        } else if constexpr (Type == OpType::BINARY) {
            for (const auto& y : specialValues) {
                hub_float result = operation_(x.first, y.first);
                std::string desc = x.second + " " + opName_ + " " + y.second;
                 // Write Hex values
                 outfile_hex << x.first.toHexString().substr(2) << ","
                             << y.first.toHexString().substr(2) << ","
                             << result.toHexString().substr(2) << ","
                             << desc << "\n";
                 // Conditionally write Numeric values
                 if (outfile_num) {
                    *outfile_num << x.first << "," << y.first << "," << result << "," << desc << "\n";
                 }
            }
        } else { // UNARY
            hub_float result = operation_(x.first);
            std::string desc = opName_ + " of " + x.second;
            // Write Hex values
            outfile_hex << x.first.toHexString().substr(2) << ","
                        << result.toHexString().substr(2) << ","
                        << desc << "\n";
            // Conditionally write Numeric values
            if (outfile_num) {
                *outfile_num << x.first << "," << result << "," << desc << "\n";
            }
        }
    }

    // --- Cleanup ---
    outfile_hex.close();
    std::cout << "Special cases (Hex) results saved to: " << hex_filename << std::endl;
    if (outfile_num) {
        outfile_num->close();
        std::cout << "Special cases (Numeric) results saved to: " << num_filename << std::endl;
    }
}

template<typename Operation, OpType Type>
template<typename... Args>
void OperationTesterImpl<Operation, Type>::performTesting(const Args&... args) {

    uint64_t maxValue = Utils::getMaxValue();
    uint64_t totalCombinations;
    
    if constexpr (Type == OpType::TERNARY) {
        // Calculate maxValue^3 safely to avoid overflow
        if (maxValue > UINT32_MAX || 
            maxValue > UINT64_MAX / maxValue ||
            maxValue * maxValue > UINT64_MAX / maxValue) {
            totalCombinations = UINT64_MAX;
        } else {
            totalCombinations = maxValue * maxValue * maxValue;
        }
    } else if constexpr (Type == OpType::BINARY) {
        if (maxValue > UINT64_MAX / maxValue) {
            totalCombinations = UINT64_MAX;
        } else {
            totalCombinations = maxValue * maxValue;
        }
    } else {
        totalCombinations = maxValue;
    }
    
    bool useSampling = TestConfig::MAX_EXHAUSTIVE_TESTS != -1 && 
                      totalCombinations > TestConfig::MAX_EXHAUSTIVE_TESTS;

    // --- File Setup ---
    std::string hex_filename = Utils::generateFilename(opName_, useSampling, false, false); // Hex file is default
    std::ofstream outfile_hex = Utils::openOutputFile(hex_filename);
    outfile_hex << std::setprecision(std::numeric_limits<long double>::max_digits10);

    std::optional<std::ofstream> outfile_num;
    std::string num_filename;
    if (TestConfig::OUTPUT_SEPARATE_NUMERIC_FILE) {
        num_filename = Utils::generateFilename(opName_, useSampling, false, true); // Get numeric filename
        outfile_num.emplace(Utils::openOutputFile(num_filename)); // Create and open the numeric file stream
        *outfile_num << std::setprecision(std::numeric_limits<long double>::max_digits10);
    }

    // --- Header Writing ---
    // Hex Header
    if constexpr (Type == OpType::TERNARY) outfile_hex << "X,Y,Z,R\n";
    else if constexpr (Type == OpType::BINARY) outfile_hex << "X,Y,Z\n";
    else outfile_hex << "X,Z\n";
    
    // Numeric Header (Optional)
    if (outfile_num) {
        if constexpr (Type == OpType::TERNARY) *outfile_num << "X_num,Y_num,Z_num,R_num\n";
        else if constexpr (Type == OpType::BINARY) *outfile_num << "X_num,Y_num,Z_num\n";
        else *outfile_num << "X_num,Z_num\n";
    }
    
    std::cout << "Total combinations: " << totalCombinations << std::endl;
    std::cout << "Max exhaustive: " << TestConfig::MAX_EXHAUSTIVE_TESTS << std::endl;

    uint64_t sampleSize = useSampling ? TestConfig::RANDOM_SAMPLE_SIZE : totalCombinations;

    Utils::clearScreen();
    std::cout << "=== Testing " << opName_ << " Operation ===\n";
    std::cout << (useSampling ? "Using random sampling\n" : "Performing exhaustive testing\n");

    std::uniform_int_distribution<uint64_t> dist(0, maxValue - 1);
    
    // --- Data Writing ---
    if (!useSampling) {
        // --- Exhaustive Testing ---
        if constexpr (Type == OpType::TERNARY) {
            for (uint64_t x = 0; x < maxValue; ++x) {
                for (uint64_t y = 0; y < maxValue; ++y) {
                    for (uint64_t z = 0; z < maxValue; ++z) {
                        hub_float value1(static_cast<uint32_t>(x));
                        hub_float value2(static_cast<uint32_t>(y));
                        hub_float value3(static_cast<uint32_t>(z));
                        hub_float result = operation_(value1, value2, value3);
                        // Write Hex values
                        outfile_hex << value1.toHexString().substr(2) << "," 
                                    << value2.toHexString().substr(2) << "," 
                                    << value3.toHexString().substr(2) << "," 
                                    << result.toHexString().substr(2) << "\n";
                        // Conditionally write Numeric values
                        if (outfile_num) {
                            *outfile_num << value1 << "," << value2 << "," << value3 << "," << result << "\n";
                        }
                        Utils::displayCalculation(value1, value2, value3, result);
                        uint64_t progress = ((x * maxValue + y) * maxValue + z) + 1;
                        Utils::showProgress(progress, totalCombinations, "Testing " + opName_);
                    }
                }
            }
        } else if constexpr (Type == OpType::BINARY) {
            for (uint64_t x = 0; x < maxValue; ++x) {
                for (uint64_t y = 0; y < maxValue; ++y) {
                    hub_float value1(static_cast<uint32_t>(x));
                    hub_float value2(static_cast<uint32_t>(y));
                    hub_float result = operation_(value1, value2);
                    // Write Hex values
                    outfile_hex << value1.toHexString().substr(2) << "," 
                                << value2.toHexString().substr(2) << "," 
                                << result.toHexString().substr(2) << "\n";
                    // Conditionally write Numeric values
                    if (outfile_num) {
                        *outfile_num << value1 << "," << value2 << "," << result << "\n";
                    }
                    Utils::displayCalculation(value1, value2, result);
                    Utils::showProgress(x * maxValue + y + 1, totalCombinations, "Testing " + opName_);
                }
            }
        } else { // UNARY
            for (uint64_t x = 0; x < maxValue; ++x) {
                hub_float value1(static_cast<uint32_t>(x));
                hub_float result = operation_(value1);
                // Write Hex values
                outfile_hex << value1.toHexString().substr(2) << "," 
                            << result.toHexString().substr(2) << "\n";
                // Conditionally write Numeric values
                if (outfile_num) {
                    *outfile_num << value1 << "," << result << "\n";
                }
                Utils::displayCalculation(value1, result);
                Utils::showProgress(x + 1, totalCombinations, "Testing " + opName_);
            }
        }
    } else { 
        // --- Random Sampling ---
        for (uint64_t i = 0; i < sampleSize; ++i) {
            uint64_t x = dist(rng_);
            hub_float value1(static_cast<uint32_t>(x));
            hub_float result;

            if constexpr (Type == OpType::TERNARY) {
                uint64_t y = dist(rng_);
                uint64_t z = dist(rng_);
                hub_float value2(static_cast<uint32_t>(y));
                hub_float value3(static_cast<uint32_t>(z));
                result = operation_(value1, value2, value3);
                // Write Hex values
                outfile_hex << value1.toHexString().substr(2) << "," 
                            << value2.toHexString().substr(2) << "," 
                            << value3.toHexString().substr(2) << "," 
                            << result.toHexString().substr(2) << "\n";
                // Conditionally write Numeric values
                if (outfile_num) {
                    *outfile_num << value1 << "," << value2 << "," << value3 << "," << result << "\n";
                }
                Utils::displayCalculation(value1, value2, value3, result);
            } else if constexpr (Type == OpType::BINARY) {
                uint64_t y = dist(rng_);
                hub_float value2(static_cast<uint32_t>(y));
                result = operation_(value1, value2);
                // Write Hex values
                outfile_hex << value1.toHexString().substr(2) << "," 
                            << value2.toHexString().substr(2) << "," 
                            << result.toHexString().substr(2) << "\n";
                // Conditionally write Numeric values
                if (outfile_num) {
                    *outfile_num << value1 << "," << value2 << "," << result << "\n";
                }
                Utils::displayCalculation(value1, value2, result);
            } else { // UNARY
                result = operation_(value1);
                // Write Hex values
                outfile_hex << value1.toHexString().substr(2) << "," 
                            << result.toHexString().substr(2) << "\n";
                // Conditionally write Numeric values
                if (outfile_num) {
                    *outfile_num << value1 << "," << result << "\n";
                }
                Utils::displayCalculation(value1, result);
            }
            Utils::showProgress(i + 1, sampleSize, "Testing " + opName_);
        }
    }

    // --- Cleanup ---
    outfile_hex.close();
    std::cout << "\nResults (Hex) saved to: " << hex_filename << std::endl;
     if (outfile_num) {
        outfile_num->close();
        std::cout << "Results (Numeric) saved to: " << num_filename << std::endl;
    }
}

template<typename Op>
std::unique_ptr<OperationTester> createTester(const std::string& name, Op operation) {
    if constexpr (std::is_invocable_r_v<hub_float, Op, hub_float, hub_float, hub_float>) {
        return std::make_unique<OperationTesterImpl<Op, OpType::TERNARY>>(name, operation);
    } else if constexpr (std::is_invocable_r_v<hub_float, Op, hub_float, hub_float>) {
        return std::make_unique<OperationTesterImpl<Op, OpType::BINARY>>(name, operation);
    } else {
        return std::make_unique<OperationTesterImpl<Op, OpType::UNARY>>(name, operation);
    }
}

// Explicit template instantiation
template class OperationTesterImpl<std::function<hub_float(const hub_float&)>, OpType::UNARY>;
template class OperationTesterImpl<std::function<hub_float(const hub_float&, const hub_float&)>, OpType::BINARY>;
template class OperationTesterImpl<std::function<hub_float(const hub_float&, const hub_float&, const hub_float&)>, OpType::TERNARY>;

// Template instantiation for createTester
template std::unique_ptr<OperationTester> createTester<std::function<hub_float(const hub_float&)>>(
    const std::string&, std::function<hub_float(const hub_float&)>);
template std::unique_ptr<OperationTester> createTester<std::function<hub_float(const hub_float&, const hub_float&)>>(
    const std::string&, std::function<hub_float(const hub_float&, const hub_float&)>);
template std::unique_ptr<OperationTester> createTester<std::function<hub_float(const hub_float&, const hub_float&, const hub_float&)>>(
    const std::string&, std::function<hub_float(const hub_float&, const hub_float&, const hub_float&)>);