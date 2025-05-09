/*
Licensed under the MIT License given below.
Copyright 2023 Daniel Lidstrom
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#if !defined(NEURAL_H)
#define NEURAL_H

#include <functional>
#include <vector>

namespace Neural {
    template<typename T>
    using Vector_t = std::vector<T>;
    
    using Vector = Vector_t<double>;
    using Matrix = std::vector<Vector>;
    
    template<typename T>
    struct Network_t {
        size_t inputCount;
        size_t hiddenCount;
        size_t outputCount;
        Vector_t<T> weightsHidden;
        Vector_t<T> biasesHidden;
        Vector_t<T> weightsOutput;
        Vector_t<T> biasesOutput;
        
        Vector_t<T> Predict(const Vector_t<T>& input) const;
        Vector_t<T> Predict(const Vector_t<T>& input, Vector_t<T>& hidden, Vector_t<T>& output) const;
        
        // Convert from a double-based network to a custom type network
        static Network_t<T> FromDouble(const Network_t<double>& doubleNetwork) {
            Network_t<T> network;
            network.inputCount = doubleNetwork.inputCount;
            network.hiddenCount = doubleNetwork.hiddenCount;
            network.outputCount = doubleNetwork.outputCount;
            
            // Convert weights and biases
            network.weightsHidden.resize(doubleNetwork.weightsHidden.size());
            for (size_t i = 0; i < doubleNetwork.weightsHidden.size(); i++) {
                network.weightsHidden[i] = static_cast<T>(doubleNetwork.weightsHidden[i]);
            }
            
            network.biasesHidden.resize(doubleNetwork.biasesHidden.size());
            for (size_t i = 0; i < doubleNetwork.biasesHidden.size(); i++) {
                network.biasesHidden[i] = static_cast<T>(doubleNetwork.biasesHidden[i]);
            }
            
            network.weightsOutput.resize(doubleNetwork.weightsOutput.size());
            for (size_t i = 0; i < doubleNetwork.weightsOutput.size(); i++) {
                network.weightsOutput[i] = static_cast<T>(doubleNetwork.weightsOutput[i]);
            }
            
            network.biasesOutput.resize(doubleNetwork.biasesOutput.size());
            for (size_t i = 0; i < doubleNetwork.biasesOutput.size(); i++) {
                network.biasesOutput[i] = static_cast<T>(doubleNetwork.biasesOutput[i]);
            }
            
            return network;
        }
    };
    
    // Define the classic double-based network for backward compatibility
    using Network = Network_t<double>;

    template<>
    Vector Network_t<double>::Predict(const Vector& input) const;

    template<>
    Vector Network_t<double>::Predict(const Vector& input, Vector& hidden, Vector& output) const;

    struct Trainer {
        Network network;
        Vector hidden;
        Vector output;
        Vector gradHidden;
        Vector gradOutput;
        static Trainer Create(Neural::Network&& network, size_t hiddenCount, size_t outputCount);
        static Trainer Create(size_t inputCount, size_t hiddenCount, size_t outputCount, std::function<double()> rand);
        void Train(const Vector& input, const Vector& output, double lr);
    };
}

// Template implementation
#include "neural_impl.hpp"

#endif
