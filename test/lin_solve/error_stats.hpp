#ifndef ERROR_STATS_HPP
#define ERROR_STATS_HPP

#include <limits>

// Structure to hold error statistics for linear system solvers
struct ErrorStats {
    double avg_error;         // Average absolute error
    double max_error;         // Maximum absolute error
    double min_error;         // Minimum absolute error
    double relative_error;    // Relative error (normalized by reference solution)
    double variance;          // Variance of the errors
    
    // Default constructor with initialized values
    ErrorStats() : 
        avg_error(0.0), 
        max_error(0.0), 
        min_error(std::numeric_limits<double>::max()), 
        relative_error(0.0), 
        variance(0.0) {}
    
    // Constructor with all fields
    ErrorStats(double avg, double max, double min, double rel, double var) : 
        avg_error(avg), 
        max_error(max), 
        min_error(min), 
        relative_error(rel), 
        variance(var) {}
};

#endif // ERROR_STATS_HPP
