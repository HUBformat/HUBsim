#include "operation_tester.hpp"
#include "test_config.hpp" // Added to include TestConfig
#include "utils.hpp"       // Added to include Utils
#include <functional>  // For std::function
#include <thread>

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
template<typename Operation, bool IsBinary>
OperationTesterImpl<Operation, IsBinary>::OperationTesterImpl(const std::string& opName, Operation operation)
    : OperationTester(opName), operation_(operation) {}

template<typename Operation, bool IsBinary>
void OperationTesterImpl<Operation, IsBinary>::runTests() {
    performTesting();
}

template<typename Operation, bool IsBinary>
void OperationTesterImpl<Operation, IsBinary>::runSpecialCaseTests() {
    std::string filename = Utils::generateFilename(opName_, false, true);
    std::ofstream outfile = Utils::openOutputFile(filename);

    outfile << (IsBinary ? "X,Y,Z,Description\n" : "X,Z,Description\n");
    auto specialValues = getSpecialValues();

    Utils::clearScreen();
    std::cout << "=== Testing " << opName_ << " Special Cases ===\n";

    for (size_t i = 0; i < specialValues.size(); ++i) {
        const auto& x = specialValues[i];
        if constexpr (IsBinary) {
            for (const auto& y : specialValues) {
                hub_float result = operation_(x.first, y.first);
                outfile << x.first.toHexString().substr(2) << ","
                        << y.first.toHexString().substr(2) << ","
                        << result.toHexString().substr(2) << ","
                        << x.second << " " << opName_ << " " << y.second << "\n";
            }
        } else {
            hub_float result = operation_(x.first);
            outfile << x.first.toHexString().substr(2) << ","
                    << result.toHexString().substr(2) << ","
                    << opName_ << " of " << x.second << "\n";
        }
    }

    outfile.close();
    std::cout << "Special cases results saved to: " << filename << std::endl;
}

template<typename Operation, bool IsBinary>
template<typename... Args>
void OperationTesterImpl<Operation, IsBinary>::performTesting(const Args&... args) {

    uint64_t maxValue = Utils::getMaxValue();
    uint64_t totalCombinations;
    if (IsBinary) {
        if (maxValue > UINT64_MAX / maxValue) {
            // If maxValue * maxValue would overflow, set totalCombinations to UINT64_MAX
            totalCombinations = UINT64_MAX;
            maxValue = UINT64_MAX;
        } else {
            totalCombinations = maxValue * maxValue;
        }
    } else {
        totalCombinations = maxValue;
    }    
    bool useSampling = totalCombinations > TestConfig::MAX_EXHAUSTIVE_TESTS;
    std::string filename = Utils::generateFilename(opName_, useSampling);
    std::ofstream outfile = Utils::openOutputFile(filename);
    if constexpr (IsBinary) {
        outfile << "X,Y,Z\n";
    } else {
        outfile << "X,Z\n";
    }
    std::cout << "Total combinations: " << totalCombinations << std::endl;
    std::cout << "Max exhaustive: " << TestConfig::MAX_EXHAUSTIVE_TESTS << std::endl;

    /*if (useSampling) {
        std::cout << "Random sampling is enabled for this test.\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10000)); // Pause to simulate delay*/
    uint64_t sampleSize = useSampling ? TestConfig::RANDOM_SAMPLE_SIZE : totalCombinations;

    Utils::clearScreen();
    std::cout << "=== Testing " << opName_ << " Operation ===\n";
    std::cout << (useSampling ? "Using random sampling\n" : "Performing exhaustive testing\n");

    std::uniform_int_distribution<uint64_t> dist(0, maxValue - 1);
    if (!useSampling) {
        if constexpr (IsBinary) {
            for (uint64_t x = 0; x < maxValue; ++x) {
                for (uint64_t y = 0; y < maxValue; ++y) {
                    hub_float value1(static_cast<uint32_t>(x));
                    hub_float value2(static_cast<uint32_t>(y));
                    hub_float result = operation_(value1, value2);
                    std::string xHex = value1.toHexString().substr(2);
                    std::string yHex = value2.toHexString().substr(2);
                    std::string zHex = result.toHexString().substr(2);
                    outfile << xHex << "," << yHex << "," << zHex << "\n";
                    Utils::displayCalculation(value1, value2, result);
                    Utils::showProgress(x * maxValue + y + 1, totalCombinations, "Testing " + opName_);
                }
            }
        } else {
            for (uint64_t x = 0; x < maxValue; ++x) {
                hub_float value1(static_cast<uint32_t>(x));
                hub_float result = operation_(value1);
                std::string xHex = value1.toHexString().substr(2);
                std::string zHex = result.toHexString().substr(2);
                outfile << xHex << "," << zHex << "\n";
                Utils::displayCalculation(value1, result);
                Utils::showProgress(x + 1, totalCombinations, "Testing " + opName_);
            }
        }
    } else {
        for (uint64_t i = 0; i < sampleSize; ++i) {
            uint64_t x = dist(rng_);
            hub_float value1(static_cast<uint32_t>(x));
            hub_float result;
            std::string xHex, yHex, zHex;

            if constexpr (IsBinary) {
                uint64_t y = dist(rng_);
                hub_float value2(static_cast<uint32_t>(y));
                result = operation_(value1, value2);
                xHex = value1.toHexString().substr(2);
                yHex = value2.toHexString().substr(2);
                zHex = result.toHexString().substr(2);
                outfile << xHex << "," << yHex << "," << zHex << "\n";
                Utils::displayCalculation(value1, value2, result);
            } else {
                result = operation_(value1);
                xHex = value1.toHexString().substr(2);
                zHex = result.toHexString().substr(2);
                outfile << xHex << "," << zHex << "\n";
                Utils::displayCalculation(value1, result);
            }

            Utils::showProgress(i + 1, sampleSize, "Testing " + opName_);
        }
    }

    outfile.close();
    std::cout << "\nResults saved to: " << filename << std::endl;
}

template<typename Op>
std::unique_ptr<OperationTester> createTester(const std::string& name, Op operation) {
    constexpr bool isBinary = std::is_invocable_r_v<hub_float, Op, hub_float, hub_float>;
    return std::make_unique<OperationTesterImpl<Op, isBinary>>(name, operation);
}

// Explicit template instantiation
template class OperationTesterImpl<std::function<hub_float(const hub_float&)>, false>;
template class OperationTesterImpl<std::function<hub_float(const hub_float&, const hub_float&)>, true>;

// Template instantiation for createTester
template std::unique_ptr<OperationTester> createTester<std::function<hub_float(const hub_float&)>>(
    const std::string&, std::function<hub_float(const hub_float&)>);
template std::unique_ptr<OperationTester> createTester<std::function<hub_float(const hub_float&, const hub_float&)>>(
    const std::string&, std::function<hub_float(const hub_float&, const hub_float&)>);