#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>
#include "hub_float.hpp"
#include "test_config.hpp"

namespace Utils {
    void clearScreen();
    std::string generateFilename(const std::string& opName, bool isSampled, bool isSpecialCase = false);
    uint64_t getMaxValue();
    std::ofstream openOutputFile(const std::string& filename);
    void showProgress(uint64_t current, uint64_t total, const std::string& taskName = "");
    void displayCalculation(const hub_float& x, const hub_float& y, const hub_float& result);
    void displayCalculation(const hub_float& x, const hub_float& result);
}

#endif // UTILS_HPP
