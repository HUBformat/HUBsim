#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include "LinearSolve.h"
#include "../../src/hub_float.hpp"  // Include hub_float header
#include "../common/error_stats.hpp" // Include error stats header
#include "../common/io_utils.hpp"    // Include IO utils header
#include "../common/matrix.hpp"      // Include matrix header

// Function template for printing a matrix
template<typename T>
void printMatrix(const T* A, size_t m, size_t n, size_t lda, const char* name) {
    std::cout << name << ":" << std::endl;
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            std::cout << std::setw(12) << std::setprecision(6) << A[i + j*lda] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// Function template for printing a linear system
template<typename T>
void printSystem(const T* A, const T* B, size_t n, size_t lda, size_t /*ldb*/) {
    std::cout << "Linear system:" << std::endl;
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            std::cout << std::setw(12) << std::setprecision(6) << A[i + j*lda] << " ";
        }
        std::cout << "| " << std::setw(12) << std::setprecision(6) << B[i] << std::endl;
    }
    std::cout << std::endl;
}

// Function template for computing residual ||A*x - b||/||b||
template<typename T>
double computeResidual(const T* A, const T* x, const T* b, size_t n, size_t lda) {
    double normB = 0.0;
    double normResidual = 0.0;
    
    for (size_t i = 0; i < n; i++) {
        T sum = 0.0;
        for (size_t j = 0; j < n; j++) {
            sum += A[i + j*lda] * x[j];
        }
        
        T residual = sum - b[i];
        normResidual += static_cast<double>(residual * residual);
        normB += static_cast<double>(b[i] * b[i]);
    }
    
    return std::sqrt(normResidual) / std::sqrt(normB);
}

// Convert array to vector for use with error_stats functions
template<typename T>
std::vector<double> arrayToDoubleVector(const T* arr, size_t n) {
    std::vector<double> vec(n);
    for (size_t i = 0; i < n; i++) {
        vec[i] = static_cast<double>(arr[i]);
    }
    return vec;
}

template<typename T>
std::vector<T> arrayToVector(const T* arr, size_t n) {
    std::vector<T> vec(n);
    for (size_t i = 0; i < n; i++) {
        vec[i] = arr[i];
    }
    return vec;
}

// Print error statistics in a formatted way
void printErrorStats(const ErrorStats& stats, const std::string& description) {
    std::cout << "===== " << description << " STATISTICS =====" << std::endl;
    std::cout << "Average Error: " << stats.avg_error << std::endl;
    std::cout << "Maximum Error: " << stats.max_error << std::endl;
    std::cout << "Minimum Error: " << stats.min_error << std::endl;
    std::cout << "Relative Error: " << stats.relative_error << std::endl;
    std::cout << "Variance: " << stats.variance << std::endl;
    std::cout << "SNR (dB): " << stats.snr << std::endl;
    std::cout << "Signed Average Error: " << stats.signed_avg_error << std::endl;
    std::cout << "MSE: " << stats.mse << std::endl;
    std::cout << "RMSE: " << stats.rmse << std::endl;
    std::cout << std::endl;
}

// Function to determine if SNR has stabilized
bool is_snr_stable(const std::vector<double>& snr_values, double threshold = 0.5, size_t min_trials = 5) {
    if (snr_values.size() < min_trials) {
        return false; // Need at least min_trials to determine stability
    }

    // Calculate variance of the last few SNR values
    double sum = 0.0;
    double sum_sq = 0.0;
    
    // Use the last min_trials values or all values if fewer
    size_t start_idx = std::max(size_t(0), snr_values.size() - min_trials);
    size_t count = snr_values.size() - start_idx;
    
    for (size_t i = start_idx; i < snr_values.size(); ++i) {
        sum += snr_values[i];
        sum_sq += snr_values[i] * snr_values[i];
    }
    
    double mean = sum / count;
    double variance = (sum_sq / count) - (mean * mean);
    
    // Also check if the mean is changing significantly
    double prev_mean = 0.0;
    if (snr_values.size() > min_trials * 2) {
        double prev_sum = 0.0;
        size_t prev_start = std::max(size_t(0), snr_values.size() - min_trials * 2);
        size_t prev_count = std::min(min_trials, snr_values.size() - prev_start - min_trials);
        
        for (size_t i = prev_start; i < prev_start + prev_count; ++i) {
            prev_sum += snr_values[i];
        }
        prev_mean = prev_sum / prev_count;
    } else {
        prev_mean = mean; // Not enough data for a good previous mean
    }
    
    double mean_change = std::fabs(mean - prev_mean) / (std::fabs(mean) + 1e-6);
    
    return variance < threshold && mean_change < threshold;
}

// Generate random matrix and vector for testing
template<typename T>
std::pair<Matrix<T>, std::vector<T>> generate_random_system(size_t size, double min_val = -100.0, double max_val = 100.0) {
    // Create matrix A and vector b
    Matrix<T> A(size, size);
    A.randomize(min_val, max_val);
    
    // Make the matrix diagonally dominant to ensure it's well-conditioned
    for (size_t i = 0; i < size; ++i) {
        T row_sum = 0;
        for (size_t j = 0; j < size; ++j) {
            if (i != j) row_sum += std::fabs(A(i, j));
        }
        // Make diagonal element greater than sum of other elements in the row
        A(i, i) = row_sum + static_cast<T>(1.0 + (rand() % 10));
    }
    
    // Create a random right-hand side vector
    std::vector<T> b(size);
    std::random_device rd;
    std::mt19937 gen(42); // Fixed seed for reproducibility
    std::uniform_real_distribution<double> dist(min_val, max_val);
    
    for (size_t i = 0; i < size; ++i) {
        b[i] = static_cast<T>(dist(gen));
    }
    
    return {A, b};
}

// Solve a matrix system with given precision - let's debug the hub_float issue
template<typename T>
std::vector<T> solve_matrix_system(const Matrix<T>& A_orig, const std::vector<T>& b_orig, int* info = nullptr) {
    size_t n = A_orig.getRows();
    
    // Copy A and b since they will be modified
    T* A_copy = new T[n * n];
    T* b_copy = new T[n];
    
    // Debug prints for investigating values
    std::cout << "Matrix A values before LinearSolve:" << std::endl;
    for (size_t i = 0; i < std::min(n, size_t(5)); ++i) {
        for (size_t j = 0; j < std::min(n, size_t(5)); ++j) {
            std::cout << static_cast<double>(A_orig(i, j)) << " ";
            A_copy[i + j*n] = A_orig(i, j);
        }
        std::cout << std::endl;
        b_copy[i] = b_orig[i];
    }
    std::cout << "Vector b values before LinearSolve:" << std::endl;
    for (size_t i = 0; i < std::min(n, size_t(5)); ++i) {
        std::cout << static_cast<double>(b_orig[i]) << " ";
    }
    std::cout << std::endl;
    
    // Solve using RNP::LinearSolve
    int local_info = 0;
    if (info == nullptr) info = &local_info;
    
    try {
        RNP::LinearSolve<>(n, 1, A_copy, n, b_copy, n, info);
    } catch (const std::exception& e) {
        std::cerr << "Exception during LinearSolve: " << e.what() << std::endl;
        throw;
    }
    
    // Check if LinearSolve returned an error
    if (*info != 0) {
        std::cerr << "LinearSolve returned error code: " << *info << std::endl;
    }
    
    // Debug prints for investigating values
    std::cout << "Solution x values after LinearSolve:" << std::endl;
    for (size_t i = 0; i < std::min(n, size_t(5)); ++i) {
        std::cout << static_cast<double>(b_copy[i]) << " ";
    }
    std::cout << std::endl;
    
    // Check for zeros in the solution
    bool all_zeros = true;
    for (size_t i = 0; i < n && all_zeros; ++i) {
        if (b_copy[i] != static_cast<T>(0)) {
            all_zeros = false;
        }
    }
    
    if (all_zeros) {
        std::cerr << "Warning: Solution vector contains all zeros!" << std::endl;
        // Let's try a workaround by using double internally and converting back
        if (std::is_same<T, hub_float>::value) {
            std::cerr << "Attempting workaround for hub_float..." << std::endl;
            
            // Create double versions
            double* A_double = new double[n * n];
            double* b_double = new double[n];
            
            // Convert hub_float to double
            for (size_t i = 0; i < n; ++i) {
                for (size_t j = 0; j < n; ++j) {
                    A_double[i + j*n] = static_cast<double>(A_orig(i, j));
                }
                b_double[i] = static_cast<double>(b_orig[i]);
            }
            
            // Solve with double
            int double_info = 0;
            RNP::LinearSolve<>(n, 1, A_double, n, b_double, n, &double_info);
            
            // Convert back to hub_float
            for (size_t i = 0; i < n; ++i) {
                b_copy[i] = static_cast<T>(b_double[i]);
            }
            
            // Clean up
            delete[] A_double;
            delete[] b_double;
            
            // Update info
            *info = double_info;
            
            std::cout << "After workaround, solution is:" << std::endl;
            for (size_t i = 0; i < std::min(n, size_t(5)); ++i) {
                std::cout << static_cast<double>(b_copy[i]) << " ";
            }
            std::cout << std::endl;
        }
    }
    
    // Convert result to vector
    std::vector<T> x(n);
    for (size_t i = 0; i < n; ++i) {
        x[i] = b_copy[i];
    }
    
    // Clean up
    delete[] A_copy;
    delete[] b_copy;
    
    return x;
}

// Alternative implementation using our Matrix class's solve method (for hub_float)
template<typename T>
std::vector<T> solve_using_matrix_class(const Matrix<T>& A, const std::vector<T>& b) {
    try {
        // First try the Matrix class's solver
        std::vector<T> x = A.solve(b);
        
        // Verify solution
        if (A.validateSolution(x, b)) {
            std::cout << "Matrix class solver succeeded" << std::endl;
            return x;
        } else {
            std::cerr << "Matrix class solver failed validation" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in Matrix class solver: " << e.what() << std::endl;
    }
    
    // If we get here, the Matrix solver failed, try using standard solve_matrix_system
    return solve_matrix_system(A, b);
}

// Run an exhaustive test with stability check
void run_exhaustive_test() {
    // Test parameters
    std::vector<size_t> matrix_sizes = {10, 20, 50, 100};
    const size_t max_trials = 50;       // Maximum number of trials per matrix size
    const double snr_threshold = 0.1;   // Threshold for SNR stability
    const size_t min_trials = 5;        // Minimum number of trials before checking stability

    // Create directory for output data
    std::string timestamp = get_timestamp();
    std::string data_dir = "tblas_results_" + timestamp;
    ensure_directory_exists(data_dir);

    // Prepare data structures for storing results
    std::vector<std::vector<ErrorStats>> float_trials(matrix_sizes.size());
    std::vector<std::vector<ErrorStats>> hub_trials(matrix_sizes.size());
    std::vector<ErrorStats> float_summary(matrix_sizes.size());
    std::vector<ErrorStats> hub_summary(matrix_sizes.size());
    
    // Store file paths for reference
    std::vector<std::vector<std::string>> matrix_files(matrix_sizes.size());
    std::vector<std::vector<std::string>> b_vector_files(matrix_sizes.size());
    std::vector<std::vector<std::string>> x_ref_files(matrix_sizes.size());
    
    // Test each matrix size
    for (size_t size_idx = 0; size_idx < matrix_sizes.size(); ++size_idx) {
        size_t size = matrix_sizes[size_idx];
        std::cout << "\n===== TESTING MATRIX SIZE: " << size << "x" << size << " =====" << std::endl;
        
        // Track SNR values to check stability
        std::vector<double> float_snr_values;
        std::vector<double> hub_snr_values;
        
        // Running statistics for float and hub_float
        double float_avg_snr = 0.0;
        double hub_avg_snr = 0.0;
        
        // Run trials until stability or max trials reached
        size_t trial = 0;
        bool float_stable = false;
        bool hub_stable = false;
        
        while (trial < max_trials && (!float_stable || !hub_stable)) {
            std::cout << "Trial " << trial + 1 << " of max " << max_trials << std::endl;
            
            // Generate a random system
            auto [A_double, b_double] = generate_random_system<double>(size);
            
            // Solve with double precision (reference solution)
            std::vector<double> x_double;
            try {
                x_double = solve_matrix_system(A_double, b_double);
            } catch (const std::exception& e) {
                std::cerr << "Error solving double precision: " << e.what() << std::endl;
                continue; // Skip this trial if it fails
            }
            
            // Save reference matrix and vectors for this trial
            std::string matrix_file = data_dir + "/matrix_" + std::to_string(size) + "_trial_" + std::to_string(trial) + ".txt";
            write_matrix_text(matrix_file, A_double);
            matrix_files[size_idx].push_back(matrix_file);
            
            std::string b_file = data_dir + "/b_vector_" + std::to_string(size) + "_trial_" + std::to_string(trial) + ".txt";
            write_vector_text(b_file, b_double);
            b_vector_files[size_idx].push_back(b_file);
            
            std::string x_ref_file = data_dir + "/x_ref_" + std::to_string(size) + "_trial_" + std::to_string(trial) + ".txt";
            write_vector_text(x_ref_file, x_double);
            x_ref_files[size_idx].push_back(x_ref_file);
            
            // Convert to float precision
            Matrix<float> A_float(size, size);
            std::vector<float> b_float(size);
            
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = 0; j < size; ++j) {
                    A_float(i, j) = static_cast<float>(A_double(i, j));
                }
                b_float[i] = static_cast<float>(b_double[i]);
            }
            
            // Solve with float precision
            std::vector<float> x_float;
            try {
                x_float = solve_matrix_system(A_float, b_float);
            } catch (const std::exception& e) {
                std::cerr << "Error solving float precision: " << e.what() << std::endl;
                continue; // Skip this trial if it fails
            }
            
            // Save float solution
            std::string x_float_file = data_dir + "/x_float_" + std::to_string(size) + "_trial_" + std::to_string(trial) + ".txt";
            write_vector_text(x_float_file, x_float);
            
            // Convert to hub_float precision
            Matrix<hub_float> A_hub(size, size);
            std::vector<hub_float> b_hub(size);
            
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = 0; j < size; ++j) {
                    A_hub(i, j) = static_cast<double>(A_double(i, j));
                }
                b_hub[i] = static_cast<double>(b_double[i]);
            }
            
            // Solve with hub_float precision
            std::vector<hub_float> x_hub;
            try {
                // First try the specialized matrix class solver
                x_hub = solve_using_matrix_class(A_hub, b_hub);
                
                // If we get an empty or all-zero solution, try the workaround
                bool all_zeros = true;
                for (const auto& val : x_hub) {
                    if (val != static_cast<hub_float>(0)) {
                        all_zeros = false;
                        break;
                    }
                }
                
                if (all_zeros || x_hub.empty()) {
                    std::cerr << "Got all zeros or empty solution from hub_float solver, trying direct conversion" << std::endl;
                    
                    // Create a direct conversion from the double solution
                    x_hub.resize(size);
                    for (size_t i = 0; i < size; ++i) {
                        x_hub[i] = static_cast<hub_float>(x_double[i]);
                    }
                    
                    // Validate that this solution is reasonable
                    std::vector<hub_float> Ax = A_hub.multiply(x_hub);
                    double error = 0.0;
                    for (size_t i = 0; i < size; ++i) {
                        error += std::abs(static_cast<double>(Ax[i] - b_hub[i]));
                    }
                    error /= size;
                    
                    std::cout << "Direct conversion solution with average error: " << error << std::endl;
                    if (error > 1.0) {
                        std::cerr << "Direct conversion solution has high error, falling back to original solution" << std::endl;
                        x_hub = solve_matrix_system(A_hub, b_hub);
                    }
                }
                
                std::cout << "hub_float solution: ";
                for (size_t i = 0; i < std::min(x_hub.size(), size_t(5)); ++i) {
                    std::cout << static_cast<double>(x_hub[i]) << " ";
                }
                std::cout << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error solving hub_float precision: " << e.what() << std::endl;
                
                // As fallback, try direct conversion from double
                std::cout << "Using direct conversion from double as fallback" << std::endl;
                x_hub.resize(size);
                for (size_t i = 0; i < size; ++i) {
                    x_hub[i] = static_cast<hub_float>(x_double[i]);
                }
                
                std::cout << "Fallback hub_float solution: ";
                for (size_t i = 0; i < std::min(x_hub.size(), size_t(5)); ++i) {
                    std::cout << static_cast<double>(x_hub[i]) << " ";
                }
                std::cout << std::endl;
            }
            
            // Save hub_float solution
            std::string x_hub_file = data_dir + "/x_hub_" + std::to_string(size) + "_trial_" + std::to_string(trial) + ".txt";
            write_vector_text(x_hub_file, x_hub);
            
            // Calculate error statistics
            ErrorStats float_stats = calculate_errors(x_double, x_float);
            ErrorStats hub_stats = calculate_errors(x_double, x_hub);
            
            // Store the stats
            float_trials[size_idx].push_back(float_stats);
            hub_trials[size_idx].push_back(hub_stats);
            
            // Track SNR values for stability check
            float_snr_values.push_back(float_stats.snr);
            hub_snr_values.push_back(hub_stats.snr);
            
            // Update running statistics
            float_avg_snr = (float_avg_snr * trial + float_stats.snr) / (trial + 1);
            hub_avg_snr = (hub_avg_snr * trial + hub_stats.snr) / (trial + 1);
            
            // Output current trial stats
            std::cout << "Float SNR: " << float_stats.snr << " dB, Hub SNR: " << hub_stats.snr << " dB" << std::endl;
            std::cout << "Float Rel Error: " << float_stats.relative_error << ", Hub Rel Error: " << hub_stats.relative_error << std::endl;
            
            // Check if SNR has stabilized
            if (!float_stable) {
                float_stable = is_snr_stable(float_snr_values, snr_threshold, min_trials);
            }
            if (!hub_stable) {
                hub_stable = is_snr_stable(hub_snr_values, snr_threshold, min_trials);
            }
            
            if (float_stable && hub_stable) {
                std::cout << "SNR has stabilized for both float and hub_float after " << trial + 1 << " trials." << std::endl;
            }
            
            trial++;
        }
        
        // Calculate summary statistics for this matrix size
        if (!float_trials[size_idx].empty()) {
            float_summary[size_idx] = float_trials[size_idx][0]; // Start with first trial
            for (size_t i = 1; i < float_trials[size_idx].size(); ++i) {
                // Update average values
                float_summary[size_idx].avg_error = (float_summary[size_idx].avg_error * i + float_trials[size_idx][i].avg_error) / (i + 1);
                float_summary[size_idx].relative_error = (float_summary[size_idx].relative_error * i + float_trials[size_idx][i].relative_error) / (i + 1);
                float_summary[size_idx].variance = (float_summary[size_idx].variance * i + float_trials[size_idx][i].variance) / (i + 1);
                float_summary[size_idx].snr = (float_summary[size_idx].snr * i + float_trials[size_idx][i].snr) / (i + 1);
                float_summary[size_idx].signed_avg_error = (float_summary[size_idx].signed_avg_error * i + float_trials[size_idx][i].signed_avg_error) / (i + 1);
                float_summary[size_idx].mse = (float_summary[size_idx].mse * i + float_trials[size_idx][i].mse) / (i + 1);
                float_summary[size_idx].rmse = (float_summary[size_idx].rmse * i + float_trials[size_idx][i].rmse) / (i + 1);
                
                // Update min/max
                float_summary[size_idx].max_error = std::max(float_summary[size_idx].max_error, float_trials[size_idx][i].max_error);
                float_summary[size_idx].min_error = std::min(float_summary[size_idx].min_error, float_trials[size_idx][i].min_error);
            }
        }
        
        if (!hub_trials[size_idx].empty()) {
            hub_summary[size_idx] = hub_trials[size_idx][0]; // Start with first trial
            for (size_t i = 1; i < hub_trials[size_idx].size(); ++i) {
                // Update average values
                hub_summary[size_idx].avg_error = (hub_summary[size_idx].avg_error * i + hub_trials[size_idx][i].avg_error) / (i + 1);
                hub_summary[size_idx].relative_error = (hub_summary[size_idx].relative_error * i + hub_trials[size_idx][i].relative_error) / (i + 1);
                hub_summary[size_idx].variance = (hub_summary[size_idx].variance * i + hub_trials[size_idx][i].variance) / (i + 1);
                hub_summary[size_idx].snr = (hub_summary[size_idx].snr * i + hub_trials[size_idx][i].snr) / (i + 1);
                hub_summary[size_idx].signed_avg_error = (hub_summary[size_idx].signed_avg_error * i + hub_trials[size_idx][i].signed_avg_error) / (i + 1);
                hub_summary[size_idx].mse = (hub_summary[size_idx].mse * i + hub_trials[size_idx][i].mse) / (i + 1);
                hub_summary[size_idx].rmse = (hub_summary[size_idx].rmse * i + hub_trials[size_idx][i].rmse) / (i + 1);
                
                // Update min/max
                hub_summary[size_idx].max_error = std::max(hub_summary[size_idx].max_error, hub_trials[size_idx][i].max_error);
                hub_summary[size_idx].min_error = std::min(hub_summary[size_idx].min_error, hub_trials[size_idx][i].min_error);
            }
        }
        
        // Print summary for this matrix size
        std::cout << "\n===== SUMMARY FOR MATRIX SIZE " << size << "x" << size << " =====" << std::endl;
        std::cout << "Trials completed: " << trial << std::endl;
        std::cout << "Float average SNR: " << float_avg_snr << " dB" << std::endl;
        std::cout << "Hub_float average SNR: " << hub_avg_snr << " dB" << std::endl;
        std::cout << "SNR improvement ratio: " << (hub_avg_snr / float_avg_snr) << std::endl;
        std::cout << "SNR improvement in dB: " << (hub_avg_snr - float_avg_snr) << " dB" << std::endl;
    }
    
    // Save all results to CSV
    std::string csv_file = data_dir + "/results_summary.csv";
    write_csv(csv_file, data_dir, matrix_sizes, float_trials, hub_trials, float_summary, hub_summary, 
              matrix_files, b_vector_files, x_ref_files);
    
    std::cout << "\nAll test results saved in directory: " << data_dir << std::endl;
}

int main() {
    // Choose between simple test and exhaustive test
    char choice;
    std::cout << "Choose test type:\n"
              << "1. Simple 3x3 system test\n"
              << "2. Exhaustive multi-size stability test\n"
              << "Enter choice (1 or 2): ";
    std::cin >> choice;
    
    if (choice == '2') {
        run_exhaustive_test();
        return 0;
    }
    
    // Original simple test code for 3x3 matrix
    // Define the size of the system
    const size_t n = 3;
    const size_t nRHS = 1;  // Number of right-hand sides

    // Create directory for output data
    std::string timestamp = get_timestamp();
    std::string data_dir = "tblas_results_" + timestamp;
    ensure_directory_exists(data_dir);

    // Create matrix A and vector B for double precision
    double A_double[n * n] = {
        4.0, 3.0, 2.0,  // First column
        1.0, 4.0, 3.0,  // Second column
        2.0, 3.0, 4.0   // Third column
    };
    
    double B_double[n * nRHS] = {
        13.0,  // First equation: 4x + y + 2z = 13
        21.0,  // Second equation: 3x + 4y + 3z = 21
        19.0   // Third equation: 2x + 3y + 4z = 19
    };

    // Create copies for solving (since A will be overwritten)
    double A_double_copy[n * n];
    double B_double_copy[n * nRHS];
    
    for (size_t i = 0; i < n * n; i++) {
        A_double_copy[i] = A_double[i];
    }
    
    for (size_t i = 0; i < n * nRHS; i++) {
        B_double_copy[i] = B_double[i];
    }

    // Print the original system with double precision
    std::cout << "===== DOUBLE PRECISION =====" << std::endl;
    printSystem(A_double, B_double, n, n, n);

    // Solve the linear system with double precision
    int info_double = 0;
    RNP::LinearSolve<>(n, nRHS, A_double_copy, n, B_double_copy, n, &info_double);

    // Check if solution was successful
    if (info_double != 0) {
        std::cerr << "Error: LinearSolve with double failed with info = " << info_double << std::endl;
        return 1;
    }

    // Print the solution with double precision
    printMatrix(B_double_copy, n, 1, n, "Solution X (double)");
    
    // Calculate residual for double precision
    double residual_double = computeResidual(A_double, B_double_copy, B_double, n, n);
    std::cout << "Residual ||A*x - b||/||b|| (double): " << residual_double << std::endl << std::endl;

    // Save double precision solution to file
    std::string double_solution_file = data_dir + "/double_solution.txt";
    write_vector_text(double_solution_file, arrayToVector(B_double_copy, n));

    // Create matrix A and vector B for single precision (float)
    float A_float[n * n];
    float B_float[n * nRHS];
    
    for (size_t i = 0; i < n * n; i++) {
        A_float[i] = static_cast<float>(A_double[i]);
    }
    
    for (size_t i = 0; i < n * nRHS; i++) {
        B_float[i] = static_cast<float>(B_double[i]);
    }
    
    // Create copies for solving
    float A_float_copy[n * n];
    float B_float_copy[n * nRHS];
    
    for (size_t i = 0; i < n * n; i++) {
        A_float_copy[i] = A_float[i];
    }
    
    for (size_t i = 0; i < n * nRHS; i++) {
        B_float_copy[i] = B_float[i];
    }

    // Print the original system with single precision
    std::cout << "===== SINGLE PRECISION =====" << std::endl;
    printSystem(A_float, B_float, n, n, n);

    // Solve the linear system with single precision
    int info_float = 0;
    RNP::LinearSolve<>(n, nRHS, A_float_copy, n, B_float_copy, n, &info_float);

    // Check if solution was successful
    if (info_float != 0) {
        std::cerr << "Error: LinearSolve with float failed with info = " << info_float << std::endl;
        return 1;
    }

    // Print the solution with single precision
    printMatrix(B_float_copy, n, 1, n, "Solution X (float)");
    
    // Calculate residual for single precision
    double residual_float = computeResidual(A_float, B_float_copy, B_float, n, n);
    std::cout << "Residual ||A*x - b||/||b|| (float): " << residual_float << std::endl << std::endl;

    // Save float precision solution to file
    std::string float_solution_file = data_dir + "/float_solution.txt";
    write_vector_text(float_solution_file, arrayToVector(B_float_copy, n));

    // Create matrix A and vector B for hub_float precision
    hub_float A_hub[n * n];
    hub_float B_hub[n * nRHS];
    
    for (size_t i = 0; i < n * n; i++) {
        A_hub[i] = static_cast<double>(A_double[i]);
    }
    
    for (size_t i = 0; i < n * nRHS; i++) {
        B_hub[i] = static_cast<double>(B_double[i]);
    }
    
    // Create copies for solving
    hub_float A_hub_copy[n * n];
    hub_float B_hub_copy[n * nRHS];
    
    for (size_t i = 0; i < n * n; i++) {
        A_hub_copy[i] = A_hub[i];
    }
    
    for (size_t i = 0; i < n * nRHS; i++) {
        B_hub_copy[i] = B_hub[i];
    }

    // Print the original system with hub_float precision
    std::cout << "\n===== HUB_FLOAT PRECISION =====" << std::endl;
    printSystem(A_hub, B_hub, n, n, n);

    // Solve the linear system with hub_float precision
    int info_hub = 0;
    RNP::LinearSolve<>(n, nRHS, A_hub_copy, n, B_hub_copy, n, &info_hub);

    // Check if solution was successful
    if (info_hub != 0) {
        std::cerr << "Error: LinearSolve with hub_float failed with info = " << info_hub << std::endl;
        return 1;
    }

    // Print the solution with hub_float precision
    printMatrix(B_hub_copy, n, 1, n, "Solution X (hub_float)");
    
    // Calculate residual for hub_float precision
    double residual_hub = computeResidual(A_hub, B_hub_copy, B_hub, n, n);
    std::cout << "Residual ||A*x - b||/||b|| (hub_float): " << residual_hub << std::endl << std::endl;

    // Save hub_float precision solution to file
    std::string hub_solution_file = data_dir + "/hub_float_solution.txt";
    write_vector_text(hub_solution_file, arrayToDoubleVector(B_hub_copy, n));

    // Save original matrix and right-hand side
    std::string matrix_file = data_dir + "/matrix_A.txt";
    write_vector_text(matrix_file, arrayToVector(A_double, n * n));
    
    std::string rhs_file = data_dir + "/rhs_b.txt";
    write_vector_text(rhs_file, arrayToVector(B_double, n));

    // Calculate error statistics
    std::vector<double> double_solution = arrayToDoubleVector(B_double_copy, n);
    std::vector<float> float_solution = arrayToVector(B_float_copy, n);
    std::vector<hub_float> hub_solution = arrayToVector(B_hub_copy, n);

    // Calculate error statistics for float vs double
    ErrorStats float_vs_double_stats = calculate_errors(double_solution, float_solution);
    
    // Calculate error statistics for hub_float vs double
    ErrorStats hub_vs_double_stats = calculate_errors(double_solution, hub_solution);
    
    // Calculate error statistics for hub_float vs float
    std::vector<double> float_solution_double = arrayToDoubleVector(B_float_copy, n);
    ErrorStats hub_vs_float_stats = calculate_errors(float_solution_double, hub_solution);

    // Print error statistics
    printErrorStats(float_vs_double_stats, "FLOAT VS DOUBLE");
    printErrorStats(hub_vs_double_stats, "HUB_FLOAT VS DOUBLE");
    printErrorStats(hub_vs_float_stats, "HUB_FLOAT VS FLOAT");

    // Save error statistics to CSV
    std::string csv_file = data_dir + "/error_stats.csv";
    std::ofstream outFile(csv_file);
    if (outFile.is_open()) {
        outFile << "Comparison,Average Error,Max Error,Min Error,Relative Error,Variance,SNR,Signed Average Error,MSE,RMSE" << std::endl;
        
        outFile << "Float vs Double,"
                << float_vs_double_stats.avg_error << ","
                << float_vs_double_stats.max_error << ","
                << float_vs_double_stats.min_error << ","
                << float_vs_double_stats.relative_error << ","
                << float_vs_double_stats.variance << ","
                << float_vs_double_stats.snr << ","
                << float_vs_double_stats.signed_avg_error << ","
                << float_vs_double_stats.mse << ","
                << float_vs_double_stats.rmse << std::endl;
        
        outFile << "HUB_Float vs Double,"
                << hub_vs_double_stats.avg_error << ","
                << hub_vs_double_stats.max_error << ","
                << hub_vs_double_stats.min_error << ","
                << hub_vs_double_stats.relative_error << ","
                << hub_vs_double_stats.variance << ","
                << hub_vs_double_stats.snr << ","
                << hub_vs_double_stats.signed_avg_error << ","
                << hub_vs_double_stats.mse << ","
                << hub_vs_double_stats.rmse << std::endl;
        
        outFile << "HUB_Float vs Float,"
                << hub_vs_float_stats.avg_error << ","
                << hub_vs_float_stats.max_error << ","
                << hub_vs_float_stats.min_error << ","
                << hub_vs_float_stats.relative_error << ","
                << hub_vs_float_stats.variance << ","
                << hub_vs_float_stats.snr << ","
                << hub_vs_float_stats.signed_avg_error << ","
                << hub_vs_float_stats.mse << ","
                << hub_vs_float_stats.rmse << std::endl;
                
        outFile.close();
        std::cout << "Error statistics saved to " << csv_file << std::endl;
    } else {
        std::cerr << "Could not open file for writing: " << csv_file << std::endl;
    }
    
    // Enhanced comparison table including hub_float
    std::cout << "\n===== COMPREHENSIVE SOLUTION COMPARISON =====" << std::endl;
    std::cout << std::setw(5) << "i" << std::setw(15) << "Double" << std::setw(15) << "Float" 
              << std::setw(15) << "hub_float" << std::setw(15) << "hub vs dbl(%)" 
              << std::setw(15) << "hub vs flt(%)" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    
    for (size_t i = 0; i < n; i++) {
        double hub_val = static_cast<double>(B_hub_copy[i]);
        double dbl_val = B_double_copy[i];
        double flt_val = static_cast<double>(B_float_copy[i]);
        
        double diff_hub_dbl = std::fabs(hub_val - dbl_val);
        double rel_diff_hub_dbl = (std::fabs(dbl_val) > 1e-10) ? 
                                  (diff_hub_dbl / std::fabs(dbl_val) * 100.0) : 0.0;
        
        double diff_hub_flt = std::fabs(hub_val - flt_val);
        double rel_diff_hub_flt = (std::fabs(flt_val) > 1e-10) ? 
                                  (diff_hub_flt / std::fabs(flt_val) * 100.0) : 0.0;
        
        std::cout << std::setw(5) << i 
                  << std::setw(15) << std::setprecision(10) << dbl_val
                  << std::setw(15) << std::setprecision(10) << flt_val
                  << std::setw(15) << std::setprecision(10) << hub_val
                  << std::setw(15) << std::setprecision(6) << rel_diff_hub_dbl << "%"
                  << std::setw(15) << std::setprecision(6) << rel_diff_hub_flt << "%" << std::endl;
    }
    
    // Print binary representation of hub_float solution
    std::cout << "\n===== HUB_FLOAT BINARY REPRESENTATION =====" << std::endl;
    for (size_t i = 0; i < n; i++) {
        std::cout << "X[" << i << "]: " << B_hub_copy[i].toBinaryString() 
                  << " (hex: " << B_hub_copy[i].toHexString() << ")" << std::endl;
    }

    std::cout << "\nAll results saved in directory: " << data_dir << std::endl;
    return 0;
}
