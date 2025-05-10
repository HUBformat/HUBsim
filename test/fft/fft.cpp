// file fft.c
#include <math.h>
#include "fft.hpp"
#include "hub_float.hpp"

template<typename T>
void fft(T data_re[], T data_im[], const unsigned int N) {
    rearrange(data_re, data_im, N);
    compute(data_re, data_im, N);
}

// Explicit template instantiations for float, double, and hub_float
template void fft<float>(float data_re[], float data_im[], const unsigned int N);
template void fft<double>(double data_re[], double data_im[], const unsigned int N);
template void fft<hub_float>(hub_float data_re[], hub_float data_im[], const unsigned int N);

template<typename T>
void rearrange(T data_re[], T data_im[], const unsigned int N) {
    unsigned int target = 0;
    for (unsigned int position = 0; position < N; position++) {
        if (target > position) {
            const T temp_re = data_re[target];
            const T temp_im = data_im[target];
            data_re[target] = data_re[position];
            data_im[target] = data_im[position];
            data_re[position] = temp_re;
            data_im[position] = temp_im;
        }
        unsigned int mask = N;
        while (target & (mask >>= 1))
            target &= ~mask;
        target |= mask;
    }
}

// Explicit template instantiations for float and double
template void rearrange<float>(float data_re[], float data_im[], const unsigned int N);
template void rearrange<double>(double data_re[], double data_im[], const unsigned int N);

template<typename T>
void compute(T data_re[], T data_im[], const unsigned int N) {
    const double pi = -M_PI;

    for (unsigned int step = 1; step < N; step <<= 1) {
        const unsigned int jump = step << 1;
        T twiddle_re = static_cast<T>(1.0);
        T twiddle_im = static_cast<T>(0.0);
        for (unsigned int group = 0; group < step; group++) {
            for (unsigned int pair = group; pair < N; pair += jump) {
                const unsigned int match = pair + step;
                const T product_re = twiddle_re * data_re[match] - twiddle_im * data_im[match];
                const T product_im = twiddle_im * data_re[match] + twiddle_re * data_im[match];
                data_re[match] = data_re[pair] - product_re;
                data_im[match] = data_im[pair] - product_im;
                data_re[pair] += product_re;
                data_im[pair] += product_im;
            }

            // we need the factors below for the next iteration
            // if we don't iterate then don't compute
            if (group + 1 == step) {
                continue;
            }

            double angle = pi * (static_cast<double>(group) + 1) / static_cast<double>(step);
            twiddle_re = static_cast<T>(cos(angle));
            twiddle_im = static_cast<T>(sin(angle));
        }
    }
}

// Explicit template instantiations for float, double, and hub_float
template void compute<float>(float data_re[], float data_im[], const unsigned int N);
template void compute<double>(double data_re[], double data_im[], const unsigned int N);
template void compute<hub_float>(hub_float data_re[], hub_float data_im[], const unsigned int N);