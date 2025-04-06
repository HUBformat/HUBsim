#include "hub_float.hpp"
#include "matrix.hpp"
#include "error_stats.hpp"
#include "linpack.hpp"
#include "io_utils.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>

int main() {
    std::cout << std::fixed << std::setprecision(10);
    
    // Benchmark parameters
    const std::vector<size_t> matrix_sizes = {10, 20, 50, 100, 200};
    //const std::vector<size_t> matrix_sizes = {10};
    const int num_trials = 10000;  // Number of trials per matrix size
    
    std::cout << "LinPack Benchmark: hub_float vs float precision comparison\n";
    std::cout << "----------------------------------------------------------\n";
    
    std::cout << "\nMatrix\tType\t\tAvg Error\tMax Error\tMin Error\tRel Error\tVariance\n";
    std::cout << "-------------------------------------------------------------------------------------\n";
    
    std::random_device rd;
    //std::mt19937 gen(42);
    std::mt19937 gen(rd());
    
    // Create timestamp for this run
    std::string timestamp = get_timestamp();
    
    // Create directories for data
    std::string data_dir = "benchmark_data_" + timestamp;
    ensure_directory_exists(data_dir);
    
    // Vectors to store all trial results for CSV output
    std::vector<std::vector<ErrorStats>> float_trials_results(matrix_sizes.size());
    std::vector<std::vector<ErrorStats>> hub_trials_results(matrix_sizes.size());
    std::vector<ErrorStats> float_summary_results(matrix_sizes.size());
    std::vector<ErrorStats> hub_summary_results(matrix_sizes.size());
    
    // Store filenames for matrices and vectors
    std::vector<std::vector<std::string>> matrix_filenames(matrix_sizes.size());
    std::vector<std::vector<std::string>> b_vector_filenames(matrix_sizes.size());
    std::vector<std::vector<std::string>> x_ref_filenames(matrix_sizes.size());
    
    for (size_t size_idx = 0; size_idx < matrix_sizes.size(); ++size_idx) {
        size_t size = matrix_sizes[size_idx];
        // Error accumulators for multiple trials
        ErrorStats float_stats_accum;
        ErrorStats hub_stats_accum;
        
        // Reserve space for this matrix size's trial results and filenames
        float_trials_results[size_idx].reserve(num_trials);
        hub_trials_results[size_idx].reserve(num_trials);
        matrix_filenames[size_idx].reserve(num_trials);
        b_vector_filenames[size_idx].reserve(num_trials);
        x_ref_filenames[size_idx].reserve(num_trials);
        
        for (int trial = 0; trial < num_trials; ++trial) {
            // Create double precision matrices and vectors
            Matrix<double> A_double(size, size);
            A_double.randomize(-10.0, 10.0);  // Well-conditioned matrix for stability
            
            std::vector<double> x_true(size);
            std::uniform_real_distribution<double> dist(-1, 2);
            for (size_t i = 0; i < size; ++i) {
                x_true[i] = dist(gen);
            }
            
            // Generate b = A * x_true
            std::vector<double> b_double = A_double.multiply(x_true);
            
            // Print b_double vector
            /*std::cout << "b_double: ";
            for (const auto& val : b_double) {
                std::cout << val << "\n";
            }
            std::cout << std::endl;*/

            // Reference solution (should be x_true, but we solve it again for consistency)
            std::vector<double> x_ref = A_double.solve(b_double);
            
            // Validate the solution
            if (!A_double.validateSolution(x_ref, b_double)) {
                std::cerr << "Validation failed for reference solution!" << std::endl;
            }

            // Save matrix and vectors to text files
            std::string matrix_file = data_dir + "/matrix_" + std::to_string(size) + "_trial_" + std::to_string(trial) + ".txt";
            std::string b_vector_file = data_dir + "/b_vector_" + std::to_string(size) + "_trial_" + std::to_string(trial) + ".txt";
            std::string x_ref_file = data_dir + "/x_ref_" + std::to_string(size) + "_trial_" + std::to_string(trial) + ".txt";
            
            write_matrix_text(matrix_file, A_double);
            write_vector_text(b_vector_file, b_double);
            write_vector_text(x_ref_file, x_ref);
            
            // Store filenames
            matrix_filenames[size_idx].push_back(matrix_file);
            b_vector_filenames[size_idx].push_back(b_vector_file);
            x_ref_filenames[size_idx].push_back(x_ref_file);
            
            // Run benchmark for float
            ErrorStats float_stats = run_linpack<float>(A_double, b_double, x_ref);

            // Validate float solution
            Matrix<float> A_float(size, size);
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = 0; j < size; ++j) {
                    A_float(i, j) = static_cast<float>(A_double(i, j));
                }
            }
            std::vector<float> b_float(b_double.begin(), b_double.end());
            std::vector<float> x_float = A_float.solve(b_float);
            if (!A_float.validateSolution(x_float, b_float)) {
                std::cerr << "Validation failed for float solution!" << std::endl;
            }

            // Run benchmark for hub_float
            ErrorStats hub_stats = run_linpack<hub_float>(A_double, b_double, x_ref);

            // Validate hub_float solution
            Matrix<hub_float> A_hub(size, size);
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = 0; j < size; ++j) {
                    A_hub(i, j) = static_cast<hub_float>(A_double(i, j));
                }
            }
            std::vector<hub_float> b_hub(b_double.begin(), b_double.end());
            std::vector<hub_float> x_hub = A_hub.solve(b_hub);
            if (!A_hub.validateSolution(x_hub, b_hub)) {
                std::cerr << "Validation failed for hub_float solution!" << std::endl;
            }

            // Store individual trial results
            float_trials_results[size_idx].push_back(float_stats);
            hub_trials_results[size_idx].push_back(hub_stats);
            
            // Accumulate statistics
            float_stats_accum.avg_error += float_stats.avg_error;
            float_stats_accum.max_error = std::max(float_stats_accum.max_error, float_stats.max_error);
            float_stats_accum.min_error = std::min(float_stats_accum.min_error, float_stats.min_error);
            float_stats_accum.relative_error += float_stats.relative_error;
            float_stats_accum.variance += float_stats.variance;
            
            hub_stats_accum.avg_error += hub_stats.avg_error;
            hub_stats_accum.max_error = std::max(hub_stats_accum.max_error, hub_stats.max_error);
            hub_stats_accum.min_error = std::min(hub_stats_accum.min_error, hub_stats.min_error);
            hub_stats_accum.relative_error += hub_stats.relative_error;
            hub_stats_accum.variance += hub_stats.variance;
        }
        
        // Average the accumulations
        float_stats_accum.avg_error /= num_trials;
        float_stats_accum.relative_error /= num_trials;
        float_stats_accum.variance /= num_trials;
        
        hub_stats_accum.avg_error /= num_trials;
        hub_stats_accum.relative_error /= num_trials;
        hub_stats_accum.variance /= num_trials;
        
        // Store summary results
        float_summary_results[size_idx] = float_stats_accum;
        hub_summary_results[size_idx] = hub_stats_accum;
        
        // Print results
        std::cout << size << "\tfloat\t\t" 
                  << float_stats_accum.avg_error << "\t" 
                  << float_stats_accum.max_error << "\t" 
                  << float_stats_accum.min_error << "\t"
                  << float_stats_accum.relative_error << "\t"
                  << float_stats_accum.variance << std::endl;
                  
        std::cout << size << "\thub_float\t" 
                  << hub_stats_accum.avg_error << "\t" 
                  << hub_stats_accum.max_error << "\t" 
                  << hub_stats_accum.min_error << "\t"
                  << hub_stats_accum.relative_error << "\t"
                  << hub_stats_accum.variance << std::endl;
        
        // Compare hub_float vs float
        double avg_error_improvement = float_stats_accum.avg_error / hub_stats_accum.avg_error;
        double rel_error_improvement = float_stats_accum.relative_error / hub_stats_accum.relative_error;
        double var_improvement = float_stats_accum.variance / hub_stats_accum.variance;
        
        std::string avg_verdict = avg_error_improvement > 1.0 ? "better" : "worse";
        std::string rel_verdict = rel_error_improvement > 1.0 ? "better" : "worse";
        std::string var_verdict = var_improvement > 1.0 ? "better" : "worse";
        
        std::cout << "hub_float is " << avg_error_improvement << "x " << avg_verdict 
                  << " in average error, " << rel_error_improvement << "x " << rel_verdict 
                  << " in relative error, and " << var_improvement << "x " << var_verdict
                  << " in error variance" << std::endl;
        std::cout << "-------------------------------------------------------------------------------------\n";
    }
    
    // Write results to CSV file with timestamp
    std::string csv_filename = "linpack_benchmark_" + timestamp + ".csv";
    write_csv(csv_filename, data_dir, matrix_sizes, float_trials_results, hub_trials_results,
              float_summary_results, hub_summary_results, matrix_filenames, b_vector_filenames, x_ref_filenames);
    
    return 0;
}
