#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <functional>
#include "utils.hpp"
#include "operation_tester.hpp"
#include "hub_float.hpp"

static std::function<hub_float(const hub_float&, const hub_float&)> addition = 
    [](const hub_float& a, const hub_float& b) { return a + b; };
static std::function<hub_float(const hub_float&, const hub_float&)> multiplication = 
    [](const hub_float& a, const hub_float& b) { return a * b; };
static std::function<hub_float(const hub_float&, const hub_float&)> division = 
    [](const hub_float& a, const hub_float& b) { return a / b; };
static std::function<hub_float(const hub_float&)> squareRoot = 
    [](const hub_float& a) { return sqrt(a); };

int main() {
    std::cout << std::setprecision(50);
    Utils::clearScreen();
    std::cout << "=== Hub Float Operation Tester ===\n"
              << "Config: EXP_BITS=" << EXP_BITS << ", MANT_BITS=" << MANT_BITS << "\n\n";

    std::vector<std::unique_ptr<OperationTester>> testers;

    testers.push_back(createTester("addition", addition));
    testers.push_back(createTester("multiplication", multiplication));
    testers.push_back(createTester("division", division));
    testers.push_back(createTester("sqrt", squareRoot));

    for (auto& tester : testers) {
        tester->runTests();
        tester->runSpecialCaseTests();
    }

    Utils::clearScreen();
    std::cout << "=== All Tests Completed ===\n";
    for (const auto& tester : testers) {
        std::cout << "✓ " << tester->getName() << "\n";
    }
    return 0;
}
