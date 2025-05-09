/*
Licensed under the MIT License given below.
Copyright 2023 Daniel Lidstrom
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#if !defined(NEURAL_IMPL_HPP)
#define NEURAL_IMPL_HPP

#include <cmath>
#include <hub_float.hpp>

namespace Neural {
    namespace {
        // Generic sigmoid implementation
        template<typename T>
        T sigmoid(T f) { 
            return T(1.0) / (T(1.0) + T(std::exp(static_cast<double>(-f)))); 
        }
        
        // Specialization for hub_float
        template<>
        hub_float sigmoid<hub_float>(hub_float f) {
            double exp_val = std::exp(-double(f));
            return hub_float(1.0) / (hub_float(1.0) + hub_float(exp_val));
        }
    }

    // Generic implementations for all other types
    template<typename T>
    Vector_t<T> Network_t<T>::Predict(const Vector_t<T>& input) const {
        Vector_t<T> y_hidden(hiddenCount);
        Vector_t<T> y_output(outputCount);
        return Predict(input, y_hidden, y_output);
    }

    template<typename T>
    Vector_t<T> Network_t<T>::Predict(const Vector_t<T>& input, Vector_t<T>& hidden, Vector_t<T>& output) const {
        for (std::size_t c = 0; c < hiddenCount; c++) {
            T sum = T(0);
            for (size_t r = 0; r < input.size(); r++) {
                sum += input[r] * weightsHidden[r * hiddenCount + c];
            }

            hidden[c] = sigmoid(sum + biasesHidden[c]);
        }

        for (size_t c = 0; c < outputCount; c++) {
            T sum = T(0);
            for (size_t r = 0; r < hiddenCount; r++) {
                sum += hidden[r] * weightsOutput[r * outputCount + c];
            }

            output[c] = sigmoid(sum + biasesOutput[c]);
        }

        return output;
    }

    // Updated method to get raw output (pre-sigmoid activations)
    template<typename T>
    Vector_t<T> GetRawOutput(const Network_t<T>& network, const Vector_t<T>& input) {
        Vector_t<T> hidden(network.hiddenCount);
        Vector_t<T> raw_output(network.outputCount);
        
        // Compute hidden layer activation (same as in Predict)
        for (std::size_t c = 0; c < network.hiddenCount; c++) {
            T sum = T(0);
            for (size_t r = 0; r < input.size(); r++) {
                sum += input[r] * network.weightsHidden[r * network.hiddenCount + c];
            }
            hidden[c] = sigmoid(sum + network.biasesHidden[c]);
        }
        
        // Compute output layer WITHOUT applying sigmoid
        for (size_t c = 0; c < network.outputCount; c++) {
            T sum = T(0);
            for (size_t r = 0; r < network.hiddenCount; r++) {
                sum += hidden[r] * network.weightsOutput[r * network.outputCount + c];
            }
            raw_output[c] = sum + network.biasesOutput[c];
        }
        
        return raw_output;
    }
}

#endif // NEURAL_IMPL_HPP
