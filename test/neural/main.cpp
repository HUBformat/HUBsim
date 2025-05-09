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

#include "neural.h"
#include "hub_float.hpp"
#include "half.hpp"  // Added include for half-precision floating point
#include "mnist_loader.h"
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>

using namespace Neural;
using half = half_float::half;  // Create an alias for easier use

namespace {
    const int EPOCHS = 5;
    const double LEARNING_RATE = 0.1;
    const int BATCH_SIZE = 100;
    const int HIDDEN_NEURONS = 128;
    
    u_int32_t P = 2147483647;
    u_int32_t A = 16807;
    u_int32_t current = 1;
    double Rand() {
        current = current * A % P;
        double result = (double)current / P;
        return result;
    }

    // Calculate accuracy (percentage of correct predictions)
    template<typename T>
    double calculateAccuracy(const Network_t<T>& network, const Matrix& inputs, const Matrix& targets) {
        size_t correct = 0;  // Changed from int to size_t
        size_t total = inputs.size();  // Changed from int to size_t
        
        for (size_t i = 0; i < total; ++i) {
            // Convert input to the appropriate type
            Vector_t<T> typedInput(inputs[i].size());
            for (size_t j = 0; j < inputs[i].size(); j++) {
                typedInput[j] = static_cast<T>(inputs[i][j]);
            }
            
            // Now use the properly typed input
            auto output = network.Predict(typedInput);
            
            // Find predicted and actual class
            size_t predicted_class = std::max_element(output.begin(), output.end()) - output.begin();
            size_t actual_class = std::max_element(targets[i].begin(), targets[i].end()) - targets[i].begin();
            
            if (predicted_class == actual_class) {
                correct++;
            }
        }
        
        return 100.0 * static_cast<double>(correct) / static_cast<double>(total);
    }
    
    // Specialized version for double to avoid unnecessary conversion
    template<>
    double calculateAccuracy<double>(const Network_t<double>& network, const Matrix& inputs, const Matrix& targets) {
        size_t correct = 0;  // Changed from int to size_t
        size_t total = inputs.size();  // Changed from int to size_t
        
        for (size_t i = 0; i < total; ++i) {
            auto output = network.Predict(inputs[i]);
            
            // Find predicted and actual class
            size_t predicted_class = std::max_element(output.begin(), output.end()) - output.begin();
            size_t actual_class = std::max_element(targets[i].begin(), targets[i].end()) - targets[i].begin();
            
            if (predicted_class == actual_class) {
                correct++;
            }
        }
        
        return 100.0 * static_cast<double>(correct) / static_cast<double>(total);
    }

    // Calculate RMSE between two output vectors
    template<typename T1, typename T2>
    double calculateRMSE(const Vector_t<T1>& output1, const Vector_t<T2>& output2) {
        double sum_squared_diff = 0.0;
        for (size_t i = 0; i < output1.size(); ++i) {
            double diff = static_cast<double>(output1[i]) - static_cast<double>(output2[i]);
            sum_squared_diff += diff * diff;
        }
        return std::sqrt(sum_squared_diff / output1.size());
    }
    
    // Calculate RMSE between network predictions with different precision types
    template<typename T1, typename T2>
    double calculateNetworkRMSE(
        const Network_t<T1>& network1, 
        const Network_t<T2>& network2, 
        const Matrix& inputs, 
        bool use_raw_output = false,  // true = pre-sigmoid, false = post-sigmoid
        int max_samples = 1000) 
    {
        double total_squared_diff = 0.0;
        int total_values = 0;
        
        int num_samples = std::min(max_samples, static_cast<int>(inputs.size()));
        
        for (int i = 0; i < num_samples; ++i) {
            // Convert input for network1
            Vector_t<T1> input1(inputs[i].size());
            for (size_t j = 0; j < inputs[i].size(); ++j) {
                input1[j] = static_cast<T1>(inputs[i][j]);
            }
            
            // Convert input for network2
            Vector_t<T2> input2(inputs[i].size());
            for (size_t j = 0; j < inputs[i].size(); ++j) {
                input2[j] = static_cast<T2>(inputs[i][j]);
            }
            
            // Get outputs
            Vector_t<T1> output1;
            Vector_t<T2> output2;
            
            if (use_raw_output) {
                output1 = GetRawOutput(network1, input1);
                output2 = GetRawOutput(network2, input2);
            } else {
                output1 = network1.Predict(input1);
                output2 = network2.Predict(input2);
            }
            
            // Calculate squared differences
            for (size_t j = 0; j < output1.size(); ++j) {
                double diff = static_cast<double>(output1[j]) - static_cast<double>(output2[j]);
                total_squared_diff += diff * diff;
            }
            
            total_values += output1.size();
        }
        
        // Calculate RMSE
        return std::sqrt(total_squared_diff / total_values);
    }
}

// Function to compare raw output values across different precision types
void compareRawOutputs(
    const Network_t<double>& doubleNetwork,
    const Network_t<half>& halfNetwork,
    const Network_t<hub_float>& hubNetwork,
    const std::vector<std::vector<double>>& images,
    const std::vector<std::vector<double>>& labels,
    int numSamples = 5)
{
    std::cout << "\n==== Comparing Raw Output Values (Pre-Sigmoid) ====\n";
    
    // Limit the number of samples to display
    numSamples = std::min(numSamples, static_cast<int>(images.size()));
    
    double totalDiffHalfDouble = 0.0;
    double totalDiffHubDouble = 0.0;
    double maxDiffHalfDouble = 0.0;
    double maxDiffHubDouble = 0.0;
    
    // For RMSE calculation per sample
    std::vector<double> rmseHalfDouble(numSamples, 0.0);
    std::vector<double> rmseHubDouble(numSamples, 0.0);
    
    for (int i = 0; i < numSamples; ++i) {
        // Prepare inputs for each precision type
        const auto& input = images[i];
        
        Vector_t<half> halfInput(input.size());
        Vector_t<hub_float> hubInput(input.size());
        for (size_t j = 0; j < input.size(); ++j) {
            halfInput[j] = static_cast<half>(input[j]);
            hubInput[j] = static_cast<hub_float>(input[j]);
        }
        
        // Get raw outputs (before sigmoid activation in output layer)
        auto rawDoubleOutput = GetRawOutput(doubleNetwork, input);
        auto rawHalfOutput = GetRawOutput(halfNetwork, halfInput);
        auto rawHubOutput = GetRawOutput(hubNetwork, hubInput);
        
        // Calculate RMSE for this sample
        rmseHalfDouble[i] = calculateRMSE(rawHalfOutput, rawDoubleOutput);
        rmseHubDouble[i] = calculateRMSE(rawHubOutput, rawDoubleOutput);
        
        // Find actual digit
        int actual_digit = std::max_element(labels[i].begin(), labels[i].end()) - labels[i].begin();
        
        std::cout << "\nSample " << i << " (Actual digit: " << actual_digit << ")\n";
        std::cout << "Class\t| Double\t\t| half\t\t\t| hub_float\t\t| half-Double\t| hub-Double\n";
        std::cout << "----------------------------------------------------------------------------------------------\n";
        
        for (size_t j = 0; j < rawDoubleOutput.size(); ++j) {
            double diffHalfDouble = std::abs(static_cast<double>(rawHalfOutput[j]) - rawDoubleOutput[j]);
            double diffHubDouble = std::abs(static_cast<double>(rawHubOutput[j]) - rawDoubleOutput[j]);
            
            totalDiffHalfDouble += diffHalfDouble;
            totalDiffHubDouble += diffHubDouble;
            
            maxDiffHalfDouble = std::max(maxDiffHalfDouble, diffHalfDouble);
            maxDiffHubDouble = std::max(maxDiffHubDouble, diffHubDouble);
            
            std::cout << j << "\t| " 
                      << std::fixed << std::setprecision(8) << rawDoubleOutput[j] << "\t| " 
                      << std::fixed << std::setprecision(8) << static_cast<double>(rawHalfOutput[j]) << "\t| " 
                      << std::fixed << std::setprecision(8) << static_cast<double>(rawHubOutput[j]) << "\t| " 
                      << std::scientific << std::setprecision(4) << diffHalfDouble << "\t| "
                      << std::scientific << std::setprecision(4) << diffHubDouble << "\n";
        }
        
        std::cout << "Sample RMSE (half-Double): " << std::scientific << rmseHalfDouble[i] << "\n";
        std::cout << "Sample RMSE (hub_float-Double): " << std::scientific << rmseHubDouble[i] << "\n";
    }
    
    // Display summary statistics
    int totalValues = numSamples * 10; // 10 output neurons
    std::cout << "\nSummary Statistics for Sample Outputs:\n";
    std::cout << "Average absolute difference (half-Double): " << std::scientific << (totalDiffHalfDouble / totalValues) << "\n";
    std::cout << "Average absolute difference (hub_float-Double): " << std::scientific << (totalDiffHubDouble / totalValues) << "\n";
    std::cout << "Maximum absolute difference (half-Double): " << std::scientific << maxDiffHalfDouble << "\n";
    std::cout << "Maximum absolute difference (hub_float-Double): " << std::scientific << maxDiffHubDouble << "\n";
    
    // Average RMSE across samples
    double avgRmseHalfDouble = 0.0;
    double avgRmseHubDouble = 0.0;
    for (int i = 0; i < numSamples; ++i) {
        avgRmseHalfDouble += rmseHalfDouble[i];
        avgRmseHubDouble += rmseHubDouble[i];
    }
    avgRmseHalfDouble /= numSamples;
    avgRmseHubDouble /= numSamples;
    
    std::cout << "Average RMSE (half-Double): " << std::scientific << avgRmseHalfDouble << "\n";
    std::cout << "Average RMSE (hub_float-Double): " << std::scientific << avgRmseHubDouble << "\n";
    
    // Calculate RMSE over a larger dataset (1000 samples or all available)
    double rawRmseHalfDouble = calculateNetworkRMSE(halfNetwork, doubleNetwork, images, true);
    double rawRmseHubDouble = calculateNetworkRMSE(hubNetwork, doubleNetwork, images, true);
    double sigmoidRmseHalfDouble = calculateNetworkRMSE(halfNetwork, doubleNetwork, images, false);
    double sigmoidRmseHubDouble = calculateNetworkRMSE(hubNetwork, doubleNetwork, images, false);
    
    std::cout << "\nRMSE over entire dataset (up to 1000 samples):\n";
    std::cout << "Raw output RMSE (half-Double): " << std::scientific << rawRmseHalfDouble << "\n";
    std::cout << "Raw output RMSE (hub_float-Double): " << std::scientific << rawRmseHubDouble << "\n";
    std::cout << "Sigmoid output RMSE (half-Double): " << std::scientific << sigmoidRmseHalfDouble << "\n";
    std::cout << "Sigmoid output RMSE (hub_float-Double): " << std::scientific << sigmoidRmseHubDouble << "\n";
}

template<typename T>
void show_weights(const Network_t<T>& network);

int main() {
    std::cout << "Loading MNIST dataset..." << std::endl;
    
    MNISTLoader train_data;
    MNISTLoader test_data;
    
    // Load training data (using a smaller subset for quick testing)
    if (!train_data.load("train-images-idx3-ubyte", "train-labels-idx1-ubyte", 10000)) {
        std::cerr << "Error loading training data" << std::endl;
        return 1;
    }
    
    // Load test data
    if (!test_data.load("t10k-images-idx3-ubyte", "t10k-labels-idx1-ubyte", 1000)) {
        std::cerr << "Error loading test data" << std::endl;
        return 1;
    }
    
    std::cout << "Training data: " << train_data.images.size() << " samples" << std::endl;
    std::cout << "Test data: " << test_data.images.size() << " samples" << std::endl;
    
    // Create neural network (784 inputs for 28x28 images, hidden layer, 10 outputs for digits 0-9)
    Trainer trainer = Trainer::Create(784, HIDDEN_NEURONS, 10, Rand);
    
    std::cout << "Training network with " << HIDDEN_NEURONS << " hidden neurons..." << std::endl;
    
    // Create indices for shuffling
    std::vector<size_t> indices(train_data.images.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::default_random_engine rng(std::chrono::system_clock::now().time_since_epoch().count());
    
    // Training loop
    for (int epoch = 0; epoch < EPOCHS; ++epoch) {
        // Shuffle indices
        std::shuffle(indices.begin(), indices.end(), rng);
        
        int batches = train_data.images.size() / BATCH_SIZE;
        double total_loss = 0.0;
        
        for (int batch = 0; batch < batches; ++batch) {
            for (int i = 0; i < BATCH_SIZE; ++i) {
                size_t idx = indices[batch * BATCH_SIZE + i];
                trainer.Train(train_data.images[idx], train_data.labels[idx], LEARNING_RATE);
            }
            
            // Calculate loss for this batch (MSE)
            if (batch % 10 == 0) {
                double batch_loss = 0.0;
                for (int i = 0; i < BATCH_SIZE; ++i) {
                    size_t idx = indices[batch * BATCH_SIZE + i];
                    Vector output = trainer.network.Predict(train_data.images[idx]);
                    for (size_t j = 0; j < output.size(); ++j) {
                        batch_loss += std::pow(output[j] - train_data.labels[idx][j], 2);
                    }
                }
                batch_loss /= (BATCH_SIZE * 10); // Mean squared error
                total_loss += batch_loss;
                
                std::cout << "Epoch " << epoch + 1 << "/" << EPOCHS 
                          << ", Batch " << batch << "/" << batches 
                          << ", Loss: " << batch_loss << std::endl;
            }
        }
        
        // Calculate accuracy on training set
        double train_accuracy = calculateAccuracy(trainer.network, train_data.images, train_data.labels);
        
        // Calculate accuracy on test set
        double test_accuracy = calculateAccuracy(trainer.network, test_data.images, test_data.labels);
        
        std::cout << "Epoch " << epoch + 1 << "/" << EPOCHS 
                  << " completed. Average loss: " << total_loss / (batches / 10)
                  << ", Training accuracy: " << train_accuracy << "%"
                  << ", Test accuracy: " << test_accuracy << "%" << std::endl;
    }
    
    // Use the original double network for results
    const Network& doubleNetwork = trainer.network;
    
    std::cout << "\nTesting with different precision types..." << std::endl;
    
    // Test with double precision
    double doubleAccuracy = calculateAccuracy(doubleNetwork, test_data.images, test_data.labels);
    std::cout << "Double precision accuracy: " << doubleAccuracy << "%" << std::endl;
    
    // Create and test half network
    Network_t<half> halfNetwork = Network_t<half>::FromDouble(doubleNetwork);
    double halfAccuracy = calculateAccuracy(halfNetwork, test_data.images, test_data.labels);
    std::cout << "Half precision accuracy: " << halfAccuracy << "%" << std::endl;
    
    // Create and test hub_float network
    Network_t<hub_float> hubNetwork = Network_t<hub_float>::FromDouble(doubleNetwork);
    double hubAccuracy = calculateAccuracy(hubNetwork, test_data.images, test_data.labels);
    std::cout << "hub_float precision accuracy: " << hubAccuracy << "%" << std::endl;
    
    // Display some sample predictions
    std::cout << "\nSample predictions (first 1000 test images):" << std::endl;

    size_t correct_double = 0, correct_half = 0, correct_hub = 0;
    for (int i = 0; i < 1000; ++i) {
        Vector output = doubleNetwork.Predict(test_data.images[i]);

        // Convert test_data.images[i] to half for halfNetwork
        Vector_t<half> halfInput(test_data.images[i].size());
        for (size_t j = 0; j < test_data.images[i].size(); ++j) {
            halfInput[j] = static_cast<half>(test_data.images[i][j]);
        }
        Vector_t<half> output_half = halfNetwork.Predict(halfInput);

        // Convert test_data.images[i] to hub_float for hubNetwork
        Vector_t<hub_float> hubInput(test_data.images[i].size());
        for (size_t j = 0; j < test_data.images[i].size(); ++j) {
            hubInput[j] = static_cast<hub_float>(test_data.images[i][j]);
        }
        Vector_t<hub_float> output_hub = hubNetwork.Predict(hubInput);

        int predicted_digit = std::max_element(output.begin(), output.end()) - output.begin();
        int predicted_digit_half = std::max_element(output_half.begin(), output_half.end()) - output_half.begin();
        int predicted_digit_hub = std::max_element(output_hub.begin(), output_hub.end()) - output_hub.begin();
        int actual_digit = std::max_element(test_data.labels[i].begin(), test_data.labels[i].end()) - test_data.labels[i].begin();
        
        if (predicted_digit == actual_digit) correct_double++;
        if (predicted_digit_half == actual_digit) correct_half++;
        if (predicted_digit_hub == actual_digit) correct_hub++;
        /*
        std::cout << "Image " << i << ": predicted = " << predicted_digit 
                  << ", actual = " << actual_digit 
                  << (predicted_digit == actual_digit ? " ✓" : " ✗") << std::endl;
        */
    }

    std::cout << "\nAccuracy over first 1000 test images:" << std::endl;
    std::cout << "Double precision: " << (100.0 * correct_double / 1000) << "%" << std::endl;
    std::cout << "Half precision: " << (100.0 * correct_half / 1000) << "%" << std::endl;
    std::cout << "hub_float precision: " << (100.0 * correct_hub / 1000) << "%" << std::endl;
    
    // Add this new code to compare raw outputs with RMSE calculations
    compareRawOutputs(doubleNetwork, halfNetwork, hubNetwork, test_data.images, test_data.labels, 5);

    return 0;
}

template<typename T>
void show_weights(const Network_t<T>& network) {
    // Function implementation remains the same
    std::cout << "WeightsHidden:\n" << std::setprecision(6);
    for (size_t i = 0; i < network.inputCount; i++) {
        for (size_t j = 0; j < network.hiddenCount; j++) {
            std::cout << network.weightsHidden[i * network.hiddenCount + j] << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "BiasesHidden:\n";
    for (auto c : network.biasesHidden) {
        std::cout << c << ' ';
    }

    std::cout << "\nWeightsOutput:\n";
    for (size_t i = 0; i < network.hiddenCount; i++) {
        for (size_t j = 0; j < network.outputCount; j++) {
            std::cout << network.weightsOutput[i * network.outputCount + j] << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "BiasesOutput:\n";
    for (auto c : network.biasesOutput) {
        std::cout << c << ' ';
    }
    std::cout << '\n';
}

// Original non-template function for backward compatibility 
void show_weights(const Network& network) {
    show_weights<double>(network);
}
