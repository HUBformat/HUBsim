#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <iomanip>
#include "fft.hpp"
#include "../common/error_stats.hpp"
#include "../common/io_utils.hpp"
#include "../../src/hub_float.hpp"

// Helper struct to hold separate real and imaginary errors for float and hub_float
struct SeparatedStats {
    ErrorStats float_stats_re;
    ErrorStats float_stats_im;
    ErrorStats hub_stats_re;
    ErrorStats hub_stats_im;
};

SeparatedStats run_fft_test(unsigned int N, std::mt19937& gen, const std::string& data_dir = "", int trial_num = -1) {
    std::vector<double> data_re_double(N), data_im_double(N);
    std::vector<float> data_re_float(N), data_im_float(N);
    std::vector<hub_float> data_re_hub(N), data_im_hub(N);

    // Generate random input data
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (unsigned int i = 0; i < N; ++i) {
        double value = dist(gen);
        data_re_double[i] = value;
        //std::cout << "Generated value: " << value << std::endl;
        data_im_double[i] = 0.0;
        data_re_float[i] = static_cast<float>(value);
        data_im_float[i] = 0.0f;
        data_re_hub[i] = hub_float(value);
        data_im_hub[i] = hub_float(0.0);
    }

    // Save input data for Mathematica if requested
    if (!data_dir.empty() && trial_num >= 0) {
        std::string input_filename = data_dir + "/fft_input_" + std::to_string(N) + "_trial_" + std::to_string(trial_num) + ".txt";
        write_complex_data_for_mathematica(input_filename, data_re_double, data_im_double);
    }

    // Perform FFT with double precision (reference)
    std::vector<double> ref_re = data_re_double, ref_im = data_im_double;
    fft(ref_re.data(), ref_im.data(), N);

    // Save reference output for Mathematica if requested
    if (!data_dir.empty() && trial_num >= 0) {
        std::string ref_output_filename = data_dir + "/fft_output_ref_" + std::to_string(N) + "_trial_" + std::to_string(trial_num) + ".txt";
        write_complex_data_for_mathematica(ref_output_filename, ref_re, ref_im);
    }

    // Perform FFT with float
    std::vector<float> result_re_float = data_re_float, result_im_float = data_im_float;
    fft(result_re_float.data(), result_im_float.data(), N);

    // Save float output for Mathematica if requested
    if (!data_dir.empty() && trial_num >= 0) {
        std::string float_output_filename = data_dir + "/fft_output_float_" + std::to_string(N) + "_trial_" + std::to_string(trial_num) + ".txt";
        write_complex_data_for_mathematica(float_output_filename, result_re_float, result_im_float);
    }

    // Perform FFT with hub_float
    std::vector<hub_float> result_re_hub = data_re_hub, result_im_hub = data_im_hub;
    fft(result_re_hub.data(), result_im_hub.data(), N);

    // Save hub_float output for Mathematica if requested
    if (!data_dir.empty() && trial_num >= 0) {
        std::string hub_output_filename = data_dir + "/fft_output_hub_" + std::to_string(N) + "_trial_" + std::to_string(trial_num) + ".txt";
        write_complex_data_for_mathematica(hub_output_filename, result_re_hub, result_im_hub);
    }

    // Calculate errors for real and imaginary parts
    ErrorStats float_stats_re = calculate_errors(ref_re, result_re_float);
    ErrorStats float_stats_im = calculate_errors(ref_im, result_im_float);
    ErrorStats hub_stats_re = calculate_errors(ref_re, result_re_hub);
    ErrorStats hub_stats_im = calculate_errors(ref_im, result_im_hub);

    SeparatedStats out {
        float_stats_re,
        float_stats_im,
        hub_stats_re,
        hub_stats_im
    };
    return out;
}

int main() {
    std::cout << std::fixed << std::setprecision(10);
    
    // FFT sizes to test (powers of 2)
    const std::vector<unsigned int> fft_sizes = {128, 256, 512, 1024, 2048, 4096};
    const int num_trials = 1000;  // Number of trials per FFT size
    
    std::cout << "FFT Benchmark: hub_float vs float precision comparison\n";
    std::cout << "----------------------------------------------------------\n";
    
    std::cout << "\nSize\tType\t\tPart\tAvg Error\tMax Error\tMin Error\tRel Error\tSNR (dB)\n";
    std::cout << "-------------------------------------------------------------------------------------\n";
    
    // Use fixed seed for reproducibility
    std::mt19937 gen(42);
    
    // Create timestamp for this run
    std::string timestamp = get_timestamp();
    
    // Create directories for data
    std::string data_dir = "fft_benchmark_data_" + timestamp;
    ensure_directory_exists(data_dir);
    
    // Vectors to store all trial results for CSV output
    std::vector<std::vector<SeparatedStats>> trials_results(fft_sizes.size());
    
    for (size_t size_idx = 0; size_idx < fft_sizes.size(); ++size_idx) {
        unsigned int size = fft_sizes[size_idx];
        
        // Error accumulators for multiple trials
        ErrorStats float_stats_re_accum, float_stats_im_accum;
        ErrorStats hub_stats_re_accum, hub_stats_im_accum;
        
        // Reserve space for this FFT size's trial results
        trials_results[size_idx].reserve(num_trials);
        
        for (int trial = 0; trial < num_trials; ++trial) {
            // Run the FFT test for this size
            // Save only a subset of results to avoid excessive files
            bool save_data = (trial < 5);  // Save only first 5 trials of each size
            SeparatedStats stats = run_fft_test(size, gen, save_data ? data_dir : "", save_data ? trial : -1);
            
            // Store individual trial results
            trials_results[size_idx].push_back(stats);
            
            // Accumulate statistics for real and imaginary parts
            float_stats_re_accum.avg_error += stats.float_stats_re.avg_error;
            float_stats_re_accum.max_error = std::max(float_stats_re_accum.max_error, stats.float_stats_re.max_error);
            float_stats_re_accum.min_error = std::min(float_stats_re_accum.min_error, stats.float_stats_re.min_error);
            float_stats_re_accum.relative_error += stats.float_stats_re.relative_error;
            float_stats_re_accum.variance += stats.float_stats_re.variance;
            float_stats_re_accum.snr += stats.float_stats_re.snr;

            float_stats_im_accum.avg_error += stats.float_stats_im.avg_error;
            float_stats_im_accum.max_error = std::max(float_stats_im_accum.max_error, stats.float_stats_im.max_error);
            float_stats_im_accum.min_error = std::min(float_stats_im_accum.min_error, stats.float_stats_im.min_error);
            float_stats_im_accum.relative_error += stats.float_stats_im.relative_error;
            float_stats_im_accum.variance += stats.float_stats_im.variance;
            float_stats_im_accum.snr += stats.float_stats_im.snr;

            hub_stats_re_accum.avg_error += stats.hub_stats_re.avg_error;
            hub_stats_re_accum.max_error = std::max(hub_stats_re_accum.max_error, stats.hub_stats_re.max_error);
            hub_stats_re_accum.min_error = std::min(hub_stats_re_accum.min_error, stats.hub_stats_re.min_error);
            hub_stats_re_accum.relative_error += stats.hub_stats_re.relative_error;
            hub_stats_re_accum.variance += stats.hub_stats_re.variance;
            hub_stats_re_accum.snr += stats.hub_stats_re.snr;

            hub_stats_im_accum.avg_error += stats.hub_stats_im.avg_error;
            hub_stats_im_accum.max_error = std::max(hub_stats_im_accum.max_error, stats.hub_stats_im.max_error);
            hub_stats_im_accum.min_error = std::min(hub_stats_im_accum.min_error, stats.hub_stats_im.min_error);
            hub_stats_im_accum.relative_error += stats.hub_stats_im.relative_error;
            hub_stats_im_accum.variance += stats.hub_stats_im.variance;
            hub_stats_im_accum.snr += stats.hub_stats_im.snr;
        }
        
        // Average the accumulations
        float_stats_re_accum.avg_error /= num_trials;
        float_stats_re_accum.relative_error /= num_trials;
        float_stats_re_accum.variance /= num_trials;
        float_stats_re_accum.snr /= num_trials;

        float_stats_im_accum.avg_error /= num_trials;
        float_stats_im_accum.relative_error /= num_trials;
        float_stats_im_accum.variance /= num_trials;
        float_stats_im_accum.snr /= num_trials;

        hub_stats_re_accum.avg_error /= num_trials;
        hub_stats_re_accum.relative_error /= num_trials;
        hub_stats_re_accum.variance /= num_trials;
        hub_stats_re_accum.snr /= num_trials;

        hub_stats_im_accum.avg_error /= num_trials;
        hub_stats_im_accum.relative_error /= num_trials;
        hub_stats_im_accum.variance /= num_trials;
        hub_stats_im_accum.snr /= num_trials;
        
        // Print results for real and imaginary parts
        std::cout << size << "\tfloat\t\treal\t" 
                << float_stats_re_accum.avg_error << "\t" 
                << float_stats_re_accum.max_error << "\t" 
                << float_stats_re_accum.min_error << "\t"
                << float_stats_re_accum.relative_error << "\t"
                << float_stats_re_accum.snr << std::endl;

        std::cout << size << "\tfloat\t\timag\t" 
                << float_stats_im_accum.avg_error << "\t" 
                << float_stats_im_accum.max_error << "\t" 
                << float_stats_im_accum.min_error << "\t"
                << float_stats_im_accum.relative_error << "\t"
                << float_stats_im_accum.snr << std::endl;

        std::cout << size << "\thub_float\treal\t" 
                << hub_stats_re_accum.avg_error << "\t" 
                << hub_stats_re_accum.max_error << "\t" 
                << hub_stats_re_accum.min_error << "\t"
                << hub_stats_re_accum.relative_error << "\t"
                << hub_stats_re_accum.snr << std::endl;

        std::cout << size << "\thub_float\timag\t" 
                << hub_stats_im_accum.avg_error << "\t" 
                << hub_stats_im_accum.max_error << "\t" 
                << hub_stats_im_accum.min_error << "\t"
                << hub_stats_im_accum.relative_error << "\t"
                << hub_stats_im_accum.snr << std::endl;

        std::cout << "-------------------------------------------------------------------------------------\n";
    }
    
    // Write results to CSV file with timestamp
    std::string csv_filename = "fft_benchmark_" + timestamp + ".csv";
    std::ofstream csv_file(csv_filename);
    
    csv_file << "FFT Size,Type,Part,Trial,Avg Error,Max Error,Min Error,Relative Error,Variance,SNR (dB)\n";
    
    // Write individual trial results
    for (size_t size_idx = 0; size_idx < fft_sizes.size(); ++size_idx) {
        unsigned int size = fft_sizes[size_idx];
        
        for (int trial = 0; trial < num_trials; ++trial) {
            const SeparatedStats& stats = trials_results[size_idx][trial];
            
            csv_file << size << ",float,real," << trial << ","
                    << stats.float_stats_re.avg_error << "," 
                    << stats.float_stats_re.max_error << "," 
                    << stats.float_stats_re.min_error << ","
                    << stats.float_stats_re.relative_error << ","
                    << stats.float_stats_re.variance << ","
                    << stats.float_stats_re.snr << "\n";

            csv_file << size << ",float,imag," << trial << ","
                    << stats.float_stats_im.avg_error << "," 
                    << stats.float_stats_im.max_error << "," 
                    << stats.float_stats_im.min_error << ","
                    << stats.float_stats_im.relative_error << ","
                    << stats.float_stats_im.variance << ","
                    << stats.float_stats_im.snr << "\n";

            csv_file << size << ",hub_float,real," << trial << ","
                    << stats.hub_stats_re.avg_error << "," 
                    << stats.hub_stats_re.max_error << "," 
                    << stats.hub_stats_re.min_error << ","
                    << stats.hub_stats_re.relative_error << ","
                    << stats.hub_stats_re.variance << ","
                    << stats.hub_stats_re.snr << "\n";

            csv_file << size << ",hub_float,imag," << trial << ","
                    << stats.hub_stats_im.avg_error << "," 
                    << stats.hub_stats_im.max_error << "," 
                    << stats.hub_stats_im.min_error << ","
                    << stats.hub_stats_im.relative_error << ","
                    << stats.hub_stats_im.variance << ","
                    << stats.hub_stats_im.snr << "\n";
        }
    }
    
    std::cout << "Results saved to " << csv_filename << std::endl;
    
    return 0;
}

