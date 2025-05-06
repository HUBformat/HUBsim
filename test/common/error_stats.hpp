#ifndef ERROR_STATS_HPP
#define ERROR_STATS_HPP

#include <vector>
#include <cmath>
#include <limits>

// Error statistics structure
struct ErrorStats {
    double avg_error = 0.0;
    double max_error = 0.0;
    double min_error = std::numeric_limits<double>::max();
    double relative_error = 0.0;  // Average relative error
    double variance = 0.0;        // Variance of errors
    double snr = 0.0;             // Signal-to-Noise Ratio
    double signed_avg_error = 0.0; // Average error with sign
    double mse = 0.0;             // Mean Squared Error
    double rmse = 0.0;            // Root Mean Squared Error
};

// Function to calculate error statistics compared to reference
template<typename T>
ErrorStats calculate_errors(const std::vector<double>& reference, const std::vector<T>& result) {
    ErrorStats stats;
    double sum_error = 0.0;
    double sum_signed_error = 0.0;
    double sum_rel_error = 0.0;
    double sum_squared_error = 0.0;  // For MSE calculation
    std::vector<double> errors;
    std::vector<double> signed_errors; // Vector of signed errors
    errors.reserve(reference.size());
    signed_errors.reserve(reference.size());
    
    // First pass: calculate basic statistics
    for (size_t i = 0; i < reference.size(); ++i) {
        double error = static_cast<double>(result[i]) - reference[i]; // Preserve sign
        signed_errors.push_back(error); // Store signed error
        errors.push_back(std::fabs(error));
        sum_error += std::fabs(error);
        sum_signed_error += error; // Accumulate signed error
        sum_squared_error += error * error;  // For MSE calculation
        stats.max_error = std::max(stats.max_error, std::fabs(error));
        stats.min_error = std::min(stats.min_error, std::fabs(error));
        
        if (std::fabs(reference[i]) > 1e-10) {
            double rel_error = std::fabs(error) / std::fabs(reference[i]);
            sum_rel_error += rel_error;
        }
    }
    
    stats.avg_error = sum_error / reference.size();
    stats.signed_avg_error = sum_signed_error / reference.size(); // Calculate signed average error
    stats.relative_error = sum_rel_error / reference.size();
    
    // Calculate MSE and RMSE
    stats.mse = sum_squared_error / reference.size();
    stats.rmse = std::sqrt(stats.mse);
    
    // Second pass: calculate variance
    double sum_squared_diff = 0.0;
    for (const double& error : errors) {
        double diff = error - stats.avg_error;
        sum_squared_diff += diff * diff;
    }
    stats.variance = sum_squared_diff / reference.size();

    // Calculate Signal-to-Noise Ratio (SNR)
    double signal_power = 0.0; // sum(Aij^2)
    double noise_power = 0.0;  // sum((Aij - Bij)^2)

    for (size_t i = 0; i < reference.size(); ++i) {
        signal_power += reference[i] * reference[i];
        noise_power += errors[i] * errors[i]; // errors[i] = Aij - Bij
    }
    
    stats.snr = 10.0 * std::log10(signal_power / noise_power);
    if (std::isinf(stats.snr)) {
        stats.snr = std::numeric_limits<double>::max();  // Set SNR to the maximum representable double value
    }    

    return stats;
}

#endif // ERROR_STATS_HPP