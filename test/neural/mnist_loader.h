/*
Licensed under the MIT License given below.
Copyright 2023
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

#ifndef MNIST_LOADER_H
#define MNIST_LOADER_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>

class MNISTLoader {
public:
    std::vector<std::vector<double>> images;
    std::vector<std::vector<double>> labels;
    
    // Load MNIST dataset from the given files
    bool load(const std::string& images_file, const std::string& labels_file, int max_samples = -1) {
        if (!readImages(images_file, max_samples)) {
            std::cerr << "Failed to load images file: " << images_file << std::endl;
            return false;
        }
        
        if (!readLabels(labels_file, max_samples)) {
            std::cerr << "Failed to load labels file: " << labels_file << std::endl;
            return false;
        }
        
        return true;
    }
    
private:
    // Read and convert big-endian integer
    uint32_t readInt(std::ifstream& file) {
        char buf[4];
        file.read(buf, 4);
        return ((uint8_t)buf[0] << 24) | 
               ((uint8_t)buf[1] << 16) | 
               ((uint8_t)buf[2] << 8) | 
               (uint8_t)buf[3];
    }
    
    // Read MNIST image file
    bool readImages(const std::string& filename, int max_samples) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return false;
        
        // Read header
        uint32_t magic = readInt(file);
        if (magic != 0x803) {
            std::cerr << "Invalid magic number in images file" << std::endl;
            return false;
        }
        
        uint32_t num_items = readInt(file);
        uint32_t rows = readInt(file);
        uint32_t cols = readInt(file);
        
        // Limit number of samples if requested
        if (max_samples > 0 && max_samples < (int)num_items) {
            num_items = max_samples;
        }
        
        // Read images
        images.resize(num_items);
        for (uint32_t i = 0; i < num_items; i++) {
            images[i].resize(rows * cols);
            for (uint32_t j = 0; j < rows * cols; j++) {
                uint8_t pixel;
                file.read((char*)&pixel, 1);
                // Normalize pixel value to [0, 1]
                images[i][j] = static_cast<double>(pixel) / 255.0;
            }
        }
        
        return true;
    }
    
    // Read MNIST label file
    bool readLabels(const std::string& filename, int max_samples) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return false;
        
        // Read header
        uint32_t magic = readInt(file);
        if (magic != 0x801) {
            std::cerr << "Invalid magic number in labels file" << std::endl;
            return false;
        }
        
        uint32_t num_items = readInt(file);
        
        // Limit number of samples if requested
        if (max_samples > 0 && max_samples < (int)num_items) {
            num_items = max_samples;
        }
        
        // Read labels and convert to one-hot encoding
        labels.resize(num_items);
        for (uint32_t i = 0; i < num_items; i++) {
            labels[i].resize(10, 0.0); // 10 possible digits (0-9)
            uint8_t label;
            file.read((char*)&label, 1);
            labels[i][label] = 1.0; // One-hot encoding
        }
        
        return true;
    }
};

#endif // MNIST_LOADER_H
