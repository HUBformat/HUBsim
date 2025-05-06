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

    // Write dimensions
    outFile << matrix.getRows() << " " << matrix.getCols() << std::endl;

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

    // Write size
    outFile << vec.size() << std::endl;

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
            << "Relative Error,Variance,SNR,Signed Average Error,MSE,RMSE,"
            << "Matrix File,B Vector File,X Ref File" << std::endl;

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
                    << stats.snr << ","
                    << stats.signed_avg_error << ","
                    << stats.mse << ","
                    << stats.rmse << ","
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
                    << stats.snr << ","
                    << stats.signed_avg_error << ","
                    << stats.mse << ","
                    << stats.rmse << ","
                    << matrix_files[i][j] << ","
                    << b_vector_files[i][j] << ","
                    << x_ref_files[i][j] << std::endl;
        }
    }
    
    // Write summary section
    outFile << std::endl << "SUMMARY" << std::endl;
    outFile << "Matrix Size,Type,Average Error,Max Error,Min Error,"
            << "Relative Error,Variance,SNR,Signed Average Error,MSE,RMSE" << std::endl;
            
    for (size_t i = 0; i < matrix_sizes.size(); ++i) {
        size_t size = matrix_sizes[i];
        
        const auto& float_stats = float_summary[i];
        outFile << size << ",float,"
                << float_stats.avg_error << ","
                << float_stats.max_error << ","
                << float_stats.min_error << ","
                << float_stats.relative_error << ","
                << float_stats.variance << ","
                << float_stats.snr << ","
                << float_stats.signed_avg_error << ","
                << float_stats.mse << ","
                << float_stats.rmse << std::endl;
                
        const auto& hub_stats = hub_summary[i];
        outFile << size << ",hub_float,"
                << hub_stats.avg_error << ","
                << hub_stats.max_error << ","
                << hub_stats.min_error << ","
                << hub_stats.relative_error << ","
                << hub_stats.variance << ","
                << hub_stats.snr << ","
                << hub_stats.signed_avg_error << ","
                << hub_stats.mse << ","
                << hub_stats.rmse << std::endl;
                
        // Add improvement metrics
        double avg_error_improvement = float_stats.avg_error / hub_stats.avg_error;
        double rel_error_improvement = float_stats.relative_error / hub_stats.relative_error;
        double var_improvement = float_stats.variance / hub_stats.variance;
        double snr_improvement = hub_stats.snr / float_stats.snr; // Higher SNR is better
        double mse_improvement = float_stats.mse / hub_stats.mse;
        double rmse_improvement = float_stats.rmse / hub_stats.rmse;
        
        outFile << size << ",improvement,"
                << avg_error_improvement << ",,,"
                << rel_error_improvement << ","
                << var_improvement << ","
                << snr_improvement << ",,"
                << mse_improvement << ","
                << rmse_improvement << std::endl << std::endl;
    }
    
    outFile.close();
    std::cout << "Results saved to " << filename << std::endl;
    std::cout << "Data files saved in " << data_dir << " directory" << std::endl;
}

// Write complex data (real and imaginary parts) to a file for Mathematica
template<typename T1, typename T2>
void write_complex_data_for_mathematica(const std::string& filename, 
                                       const std::vector<T1>& real_part, 
                                       const std::vector<T2>& imag_part) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    
    outFile << std::scientific << std::setprecision(15);
    outFile << "{" << std::endl;  // Open main list
    
    size_t n = real_part.size();
    for (size_t i = 0; i < n; ++i) {
        outFile << "  {" << static_cast<double>(real_part[i]) << ", " 
                << static_cast<double>(imag_part[i]) << "}";
        if (i < n - 1) {
            outFile << "," << std::endl;
        } else {
            outFile << std::endl;
        }
    }
    
    outFile << "}" << std::endl;  // Close main list
    outFile.close();
}

#endif // IO_UTILS_HPP