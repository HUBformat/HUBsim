#ifndef IO_UTILS_HPP
#define IO_UTILS_HPP

#include "error_stats.hpp"
#include "matrix.hpp"
#include <string>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <iomanip>

// Get current timestamp as string
inline std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
    return ss.str();
}

// Ensure directory exists, create it if it doesn't
inline void ensure_directory_exists(const std::string& dirPath) {
    namespace fs = std::filesystem;
    if (!fs::exists(dirPath)) {
        fs::create_directory(dirPath);
    }
}

// Write matrix to text file
template<typename T>
void write_matrix_text(const std::string& filename, const Matrix<T>& matrix) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    outFile << std::scientific << std::setprecision(15);
    for (size_t i = 0; i < matrix.getRows(); ++i) {
        for (size_t j = 0; j < matrix.getCols(); ++j) {
            outFile << matrix(i, j) << " ";
        }
        outFile << std::endl;
    }
    outFile.close();
}

// Write vector to text file
template<typename T>
void write_vector_text(const std::string& filename, const std::vector<T>& vec) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    outFile << std::scientific << std::setprecision(15);
    for (const auto& val : vec) {
        outFile << val << std::endl;
    }
    outFile.close();
}

// Write benchmark results to CSV file
inline void write_csv(
    const std::string& filename,
    const std::string& data_dir,
    const std::vector<size_t>& matrix_sizes,
    const std::vector<std::vector<ErrorStats>>& float_trials,
    const std::vector<std::vector<ErrorStats>>& hub_trials,
    const std::vector<ErrorStats>& float_summary,
    const std::vector<ErrorStats>& hub_summary,
    const std::vector<std::vector<std::string>>& matrix_files,
    const std::vector<std::vector<std::string>>& b_vector_files,
    const std::vector<std::vector<std::string>>& x_ref_files
) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // Write header
    outFile << "Matrix Size,Type,Trial,Average Error,Max Error,Min Error,"
            << "Relative Error,Variance,Matrix File,B Vector File,X Ref File" << std::endl;

    // Write trial data
    for (size_t i = 0; i < matrix_sizes.size(); ++i) {
        size_t size = matrix_sizes[i];
        
        for (size_t j = 0; j < float_trials[i].size(); ++j) {
            const auto& stats = float_trials[i][j];
            outFile << size << ",float," << j << ","
                    << stats.avg_error << ","
                    << stats.max_error << ","
                    << stats.min_error << ","
                    << stats.relative_error << ","
                    << stats.variance << ","
                    << matrix_files[i][j] << ","
                    << b_vector_files[i][j] << ","
                    << x_ref_files[i][j] << std::endl;
        }
        
        for (size_t j = 0; j < hub_trials[i].size(); ++j) {
            const auto& stats = hub_trials[i][j];
            outFile << size << ",hub_float," << j << ","
                    << stats.avg_error << ","
                    << stats.max_error << ","
                    << stats.min_error << ","
                    << stats.relative_error << ","
                    << stats.variance << ","
                    << matrix_files[i][j] << ","
                    << b_vector_files[i][j] << ","
                    << x_ref_files[i][j] << std::endl;
        }
    }
    
    // Write summary section
    outFile << std::endl << "SUMMARY" << std::endl;
    outFile << "Matrix Size,Type,Average Error,Max Error,Min Error,"
            << "Relative Error,Variance" << std::endl;
            
    for (size_t i = 0; i < matrix_sizes.size(); ++i) {
        size_t size = matrix_sizes[i];
        
        const auto& float_stats = float_summary[i];
        outFile << size << ",float,"
                << float_stats.avg_error << ","
                << float_stats.max_error << ","
                << float_stats.min_error << ","
                << float_stats.relative_error << ","
                << float_stats.variance << std::endl;
                
        const auto& hub_stats = hub_summary[i];
        outFile << size << ",hub_float,"
                << hub_stats.avg_error << ","
                << hub_stats.max_error << ","
                << hub_stats.min_error << ","
                << hub_stats.relative_error << ","
                << hub_stats.variance << std::endl;
                
        // Add improvement metrics
        double avg_error_improvement = float_stats.avg_error / hub_stats.avg_error;
        double rel_error_improvement = float_stats.relative_error / hub_stats.relative_error;
        double var_improvement = float_stats.variance / hub_stats.variance;
        
        outFile << size << ",improvement,"
                << avg_error_improvement << ",,,,"
                << rel_error_improvement << ","
                << var_improvement << std::endl << std::endl;
    }
    
    outFile.close();
    std::cout << "Results saved to " << filename << std::endl;
    std::cout << "Data files saved in " << data_dir << " directory" << std::endl;
}

#endif // IO_UTILS_HPP
