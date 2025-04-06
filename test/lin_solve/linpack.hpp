#ifndef LINPACK_HPP
#define LINPACK_HPP

#include "matrix.hpp"
#include "error_stats.hpp"
#include <vector>
#include <cmath>

// Function to run Linpack benchmark with different numeric types
template<typename T>
ErrorStats run_linpack(const Matrix<double>& A_double, 
                       const std::vector<double>& b_double,
                       const std::vector<double>& x_ref) {
    // Convert the inputs to the target precision
    size_t n = A_double.getRows();
    Matrix<T> A(n, n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            A(i, j) = static_cast<T>(A_double(i, j));
        }
    }
    
    std::vector<T> b(n);
    for (size_t i = 0; i < n; ++i) {
        b[i] = static_cast<T>(b_double[i]);
    }
    
    // Solve the system using the target precision
    std::vector<T> x = A.solve(b);
    
    // Calculate errors
    double sum_error = 0.0;
    double max_error = 0.0;
    double min_error = std::numeric_limits<double>::max();
    double sum_squared_error = 0.0;
    
    for (size_t i = 0; i < n; ++i) {
        double error = std::abs(static_cast<double>(x[i]) - x_ref[i]);
        sum_error += error;
        max_error = std::max(max_error, error);
        min_error = std::min(min_error, error);
        sum_squared_error += error * error;
    }
    
    double avg_error = sum_error / n;
    double variance = (sum_squared_error / n) - (avg_error * avg_error);
    
    // Calculate relative error
    double ref_norm = 0.0;
    for (size_t i = 0; i < n; ++i) {
        ref_norm += x_ref[i] * x_ref[i];
    }
    ref_norm = std::sqrt(ref_norm);
    double relative_error = sum_error / (ref_norm * n);
    
    return {avg_error, max_error, min_error, relative_error, variance};
}

#endif // LINPACK_HPP
