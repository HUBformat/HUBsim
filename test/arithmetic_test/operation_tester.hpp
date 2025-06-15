#ifndef OPERATION_TESTER_HPP
#define OPERATION_TESTER_HPP

#include <string>
#include <vector>
#include <memory>
#include <random>
#include <fstream>
#include <functional>
#include "hub_float.hpp"
#include "test_config.hpp"
#include "utils.hpp"

class OperationTester {
protected:
    std::mt19937_64 rng_{TestConfig::RANDOM_SEED};
    std::string opName_;

    std::vector<std::pair<hub_float, std::string>> getSpecialValues() const;

public:
    explicit OperationTester(std::string opName);
    virtual ~OperationTester() = default;

    virtual void runTests() = 0;
    virtual void runSpecialCaseTests() = 0;

    const std::string& getName() const;
};

// Enum to define operation type
enum class OpType {
    UNARY,
    BINARY,
    TERNARY
};

template<typename Operation, OpType Type>
class OperationTesterImpl : public OperationTester {
private:
    Operation operation_;

    template<typename... Args>
    void performTesting(const Args&... args);

public:
    OperationTesterImpl(const std::string& opName, Operation operation);
    void runTests() override;
    void runSpecialCaseTests() override;
};

template<typename Op>
std::unique_ptr<OperationTester> createTester(const std::string& name, Op operation);

#endif // OPERATION_TESTER_HPP
