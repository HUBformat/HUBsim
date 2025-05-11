#include <iostream>
#include <vector>
#include <complex>
#include <random>
#include <ctime>
#include <iomanip>
#include "hub_float.hpp"  // Include the hub_float class

// Standard Horner's rule implementation using a single template parameter
template<typename T>
T horner(const std::vector<T>& coefficients, const T& x) {
    T result = T(0);
    
    for (size_t i = 0; i < coefficients.size(); ++i) {
        result = result * x + coefficients[i];
    }
    
    return result;
}

// Function to generate a vector of random coefficients
template<typename T>
std::vector<T> generateRandomCoefficients(int count, double min, double max) {
    std::vector<T> coefficients;
    std::mt19937 gen(static_cast<unsigned int>(time(nullptr)));
    std::uniform_real_distribution<double> dist(min, max);
    
    for (int i = 0; i < count; ++i) {
        coefficients.push_back(T(dist(gen)));
    }
    
    return coefficients;
}

int main() {
    // Seed the random number generator
    std::mt19937 gen(static_cast<unsigned int>(time(nullptr)));
    
    std::cout << "=== Testing Horner's Rule with Random Coefficients ===" << std::endl;
    
    // Set parameters for random generation
    const int degree = 10;           // polynomial degree
    const double min_coef = -100.0;  // minimum coefficient value
    const double max_coef = 100.0;   // maximum coefficient value
    const double min_eval = -10.0;   // minimum evaluation point
    const double max_eval = 10.0;    // maximum evaluation point
    const int num_trials = 100000;     // number of trials to run
    
    // Counters for which type is more accurate
    int float_wins = 0;
    int hub_float_wins = 0;
    int ties = 0;
    double total_float_error = 0.0;
    double total_hub_error = 0.0;
    
    // Distributions for random generation
    std::uniform_real_distribution<double> coef_dist(min_coef, max_coef);
    std::uniform_real_distribution<double> eval_dist(min_eval, max_eval);
    
    // Run multiple trials
    for (int trial = 0; trial < num_trials; ++trial) {
        // Generate random coefficients for this trial
        std::vector<double> random_double_coeffs;
        for (int i = 0; i <= degree; ++i) {
            random_double_coeffs.push_back(coef_dist(gen));
        }
        
        // Convert to other types
        std::vector<float> random_float_coeffs;
        std::vector<hub_float> random_hub_coeffs;
        
        for (const auto& coef : random_double_coeffs) {
            random_float_coeffs.push_back(static_cast<float>(coef));
            random_hub_coeffs.push_back(hub_float(coef));
        }
        
        // Generate random evaluation point
        const double eval_point = eval_dist(gen);
        
        // Evaluate with different types
        double random_result_double = horner(random_double_coeffs, eval_point);
        float random_result_float = horner(random_float_coeffs, static_cast<float>(eval_point));
        hub_float random_result_hub = horner(random_hub_coeffs, hub_float(eval_point));
        
        // Calculate the direct value for verification (using double precision)
        double direct_result = 0.0;
        for (size_t i = 0; i < random_double_coeffs.size(); i++) {
            direct_result += random_double_coeffs[i] * std::pow(eval_point, random_double_coeffs.size() - 1 - i);
        }
        
        // Calculate absolute errors
        double float_error = std::abs(static_cast<double>(random_result_float) - random_result_double);
        double hub_error = std::abs(static_cast<double>(random_result_hub) - random_result_double);
        
        total_float_error += float_error;
        total_hub_error += hub_error;
        
        // Compare which one is more accurate
        if (float_error < hub_error) {
            float_wins++;
        } else if (hub_error < float_error) {
            hub_float_wins++;
        } else {
            ties++;
        }
        
        // Print progress every 100 trials
        if ((trial + 1) % 100 == 0) {
            std::cout << "Completed " << (trial + 1) << " trials..." << std::endl;
        }
    }
    
    // Print the results
    std::cout << "\n=== Results after " << num_trials << " trials ===" << std::endl;
    std::cout << "Float more accurate: " << float_wins << " times (" 
              << std::fixed << std::setprecision(2) << (float_wins * 100.0 / num_trials) << "%)" << std::endl;
    std::cout << "Hub_float more accurate: " << hub_float_wins << " times (" 
              << std::fixed << std::setprecision(2) << (hub_float_wins * 100.0 / num_trials) << "%)" << std::endl;
    std::cout << "Ties: " << ties << " times (" 
              << std::fixed << std::setprecision(2) << (ties * 100.0 / num_trials) << "%)" << std::endl;
    
    std::cout << "\nAverage float error: " << std::scientific << (total_float_error / num_trials) << std::endl;
    std::cout << "Average hub_float error: " << std::scientific << (total_hub_error / num_trials) << std::endl;
    std::cout << "Ratio hub_error/float_error: " << std::fixed << std::setprecision(4) 
              << (total_hub_error / total_float_error) << std::endl;
              
    // Determine overall winner
    std::cout << "\nOverall winner: ";
    if (float_wins > hub_float_wins) {
        std::cout << "Float (standard IEEE-754)" << std::endl;
    } else if (hub_float_wins > float_wins) {
        std::cout << "Hub_float" << std::endl;
    } else {
        std::cout << "Tie" << std::endl;
    }
    
    return 0;
}
