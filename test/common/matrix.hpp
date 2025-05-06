#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <vector>
#include <stdexcept>
#include <random>

// Template class for matrix operations with different numeric types
template<typename T>
class Matrix {
private:
    size_t rows, cols;
    std::vector<T> data;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c), data(r * c) {}
    
    // Accessor for elements
    T& operator()(size_t i, size_t j) { return data[i * cols + j]; }
    const T& operator()(size_t i, size_t j) const { return data[i * cols + j]; }
    
    // Get dimensions
    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }
    
    // Fill with random values
    void randomize(double min, double max) {
        std::random_device rd;
        std::mt19937 gen(42);
        std::uniform_real_distribution<double> dist(min, max);
        
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                (*this)(i, j) = static_cast<T>(dist(gen));
            }
        }
    }
    
    // Matrix-vector multiplication
    std::vector<T> multiply(const std::vector<T>& vec) const {
        if (cols != vec.size()) {
            throw std::runtime_error("Dimension mismatch in matrix-vector multiplication");
        }
        
        std::vector<T> result(rows);
        for (size_t i = 0; i < rows; ++i) {
            result[i] = T(0);
            for (size_t j = 0; j < cols; ++j) {
                result[i] += (*this)(i, j) * vec[j];
            }
        }
        return result;
    }
    
    // Matrix-matrix multiplication
    Matrix<T> multiply(const Matrix<T>& other) const {
        if (cols != other.rows) {
            throw std::runtime_error("Dimension mismatch in matrix-matrix multiplication");
        }
        
        Matrix<T> result(rows, other.cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < other.cols; ++j) {
                result(i, j) = T(0);
                for (size_t k = 0; k < cols; ++k) {
                    result(i, j) += (*this)(i, k) * other(k, j);
                }
            }
        }
        return result;
    }
    
    // LU Decomposition (simplified for benchmark)
    std::pair<Matrix<T>, Matrix<T>> lu_decomposition() const {
        if (rows != cols) {
            throw std::runtime_error("LU decomposition requires a square matrix");
        }
        
        Matrix<T> L(rows, cols);
        Matrix<T> U(rows, cols);
        
        // Initialize L and U - fix ambiguous constructor calls by using explicit static_cast
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                if (i == j) {
                    L(i, j) = static_cast<T>(1.0); // Use 1.0 for explicit double
                } else {
                    L(i, j) = static_cast<T>(0.0); // Use 0.0 for explicit double
                }
                U(i, j) = static_cast<T>(0.0); // Use 0.0 for explicit double
            }
        }
        
        // Perform LU decomposition
        for (size_t i = 0; i < rows; ++i) {
            // Upper triangular matrix
            for (size_t j = i; j < cols; ++j) {
                U(i, j) = (*this)(i, j);
                for (size_t k = 0; k < i; ++k) {
                    U(i, j) -= L(i, k) * U(k, j);
                }
            }
            
            // Lower triangular matrix
            for (size_t j = i + 1; j < rows; ++j) {
                L(j, i) = (*this)(j, i);
                for (size_t k = 0; k < i; ++k) {
                    L(j, i) -= L(j, k) * U(k, i);
                }
                L(j, i) /= U(i, i);
            }
        }
        
        return {L, U};
    }
    
    // Solve linear system Ax = b using LU decomposition
    std::vector<T> solve(const std::vector<T>& b) const {
        if (rows != cols || rows != b.size()) {
            throw std::runtime_error("Dimension mismatch in linear system solver");
        }
        
        auto [L, U] = this->lu_decomposition();
        
        // Forward substitution (Ly = b)
        std::vector<T> y(rows);
        for (size_t i = 0; i < rows; ++i) {
            y[i] = b[i];
            for (size_t j = 0; j < i; ++j) {
                y[i] -= L(i, j) * y[j];
            }
        }
        
        // Backward substitution (Ux = y)
        std::vector<T> x(rows);
        for (size_t i = rows; i-- > 0; ) {
            x[i] = y[i];
            for (size_t j = i + 1; j < rows; ++j) {
                x[i] -= U(i, j) * x[j];
            }
            x[i] /= U(i, i);
        }
        
        return x;
    }

    bool validateSolution(const std::vector<T>& x, const std::vector<T>& b, T tolerance = 0.1) const {
        if (cols != x.size() || rows != b.size()) {
            throw std::invalid_argument("Matrix dimensions don't match vector size");
        }

        std::vector<T> Ax = multiply(x);
        for (size_t i = 0; i < rows; ++i) {
            if (std::abs(Ax[i] - b[i]) > tolerance) {
                std::cout << "Ax: ";
                for (const auto& val : Ax) {
                    std::cout << val << "\n";
                }
                std::cout << "\nb: ";
                for (const auto& val : b) {
                    std::cout << val << "\n";
                }
                std::cout << std::endl;
                return false;
            }
        }
        return true;
    }


};

// Function to convert std::vector<double> to std::vector<T>
template<typename T>
std::vector<T> convert_vector(const std::vector<double>& vec) {
    std::vector<T> result(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        result[i] = static_cast<T>(vec[i]);
    }
    return result;
}

#endif // MATRIX_HPP
