#include "utils.hpp"
#include "test_config.hpp"
#include <iomanip>

namespace Utils {
    void clearScreen() {
        std::cout << "\033[2J\033[H" << std::flush;
    }

    std::string generateFilename(const std::string& opName, bool isSampled, bool isSpecialCase) {
        std::ostringstream filename;
        filename << "hub_float_" << opName 
                 << "_exp" << EXP_BITS 
                 << "_mant" << MANT_BITS;
        
        if (isSpecialCase) {
            filename << "_special_cases";
        } else if (isSampled) {
            filename << "_sampled";
        }
        filename << ".csv";
        return filename.str();
    }

    uint64_t getMaxValue() {
        const int TOTAL_BITS = 1 + EXP_BITS + MANT_BITS;
        return (1ULL << TOTAL_BITS);
    }

    std::ofstream openOutputFile(const std::string& filename) {
        std::ofstream outfile(filename);
        if (!outfile.is_open()) {
            throw std::runtime_error("Error opening output file: " + filename);
        }
        return outfile;
    }

    void showProgress(uint64_t current, uint64_t total, const std::string& taskName) {
        static auto lastUpdateTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();

        if (current > 0 && current < total &&
            std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime).count() < 100) {
            return;
        }

        lastUpdateTime = currentTime;

        double percentage = (current * 100.0) / total;
        constexpr int barWidth = 50;
        int pos = static_cast<int>(barWidth * current / static_cast<double>(total));

        std::cout << "\r" << (taskName.empty() ? "" : (taskName + ": ")) << "[";
        for (int i = 0; i < barWidth; ++i) {
            std::cout << (i < pos ? "█" : (i == pos ? "▓" : " "));
        }
        std::cout << "] " << std::fixed << std::setprecision(1) << percentage << "% "
                  << "(" << current << "/" << total << ")    ";

        if (current >= total) std::cout << " ✓\n";

        std::cout.flush();
    }

    void displayCalculation(const hub_float& x, const hub_float& y, const hub_float& result) {
        if (!TestConfig::SHOW_DETAILED_OUTPUT) return;
        std::cout << "X: " << x << " Y: " << y
                  << " Z: " << result.toHexString() << " (" << result << ")\n"
                  << "Binary: " << result.toBinaryString() << "\n";
    }

    void displayCalculation(const hub_float& x, const hub_float& result) {
        if (!TestConfig::SHOW_DETAILED_OUTPUT) return;
        std::cout << "X: " << x
                  << " Z: " << result.toHexString() << " (" << result << ")\n"
                  << "Binary: " << result.toBinaryString() << "\n";
    }
}