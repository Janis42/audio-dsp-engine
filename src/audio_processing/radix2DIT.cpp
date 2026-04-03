// attempting clean implementation of radix-2 DIT cooley tukey variant
#include <array>
#include <complex>
#include <cmath>

using namespace std::complex_literals;

// array version

std::complex<double> twiddle(int k, int N){
    double angle = -2.0 * M_PI * k / N;
    return std::polar(1.0, angle);
}

// template, flaws:
// 1. creating a function for each num_vals
// 2. creating a full copy in every call -> lots of memory to be stored
template<std::size_t num_vals>
void recursiveRadix2DIT(std::array<std::complex<double>, num_vals>& arr){
    if (num_vals==1){
        return;
    }
    
    // create two arrays of half size holding even and odd indexed values
    std::array<std::complex<double>, arr.size()/2> even;
    std::array<std::complex<double>, arr.size()/2> odd;
    for (std::size_t k{0}; k<arr.size()/2; k++){
        even[k] = arr[2*k];
        odd[k] = arr[2*k+1];
    }
    // perform fft for both partitions
    recursiveRadix2DIT(even);
    recursiveRadix2DIT(odd);
    
    for (std::size_t k {0}; k<arr.size()/2; k++){
        arr[k] = even[k] + odd[k]*twiddle(k, arr.size());
        arr[k+arr.size()/2] = even[k] - odd[k]*twiddle(k, arr.size());
    }
    return;
}


// in-place version

inline std::size_t reverseBits(std::size_t x, int bitCount)
{
    std::size_t result = 0;
    for (int i = 0; i < bitCount; ++i)
    {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}


template<std::size_t N>
void bitReversalPermute(std::array<std::complex<double>, N>& arr)
{
    int numBits = 0;
    std::size_t temp = N - 1;
    while (temp > 0)
    {
        numBits++;
        temp >>= 1;
    }

    for (std::size_t i = 0; i < N; ++i)
    {
        std::size_t j = reverseBits(i, numBits);
        if (i < j)
        {
            std::swap(arr[i], arr[j]);
        }
    }
}

/// main algorithm

template<std::size_t N>
void iterativeRadix2DIT(std::array<std::complex<double>, N>& arr)
{
    bitReversalPermute(arr);

    // butterfly updates
    for (std::size_t len = 2; len <= N; len <<= 1)
    {
        // successive factors are powers of wlen = e^{-j·2π/len}
        std::complex<double> wlen = std::polar(1.0, -2.0 * M_PI / static_cast<double>(len));

        // loop through subarrays at stage log2(N) - log2(len)
        for (std::size_t i = 0; i < N; i += len)
        {
            std::complex<double> w(1.0, 0.0);

            for (std::size_t j = 0; j < len / 2; ++j)
            {
                std::complex<double> u = arr[i + j];               // even half
                std::complex<double> t = arr[i + j + len / 2] * w; // odd half

                arr[i + j]           = u + t;
                arr[i + j + len / 2] = u - t;

                w *= wlen;
            }
        }
    }
}

// back transformation (essentially the same,
// just using conjugated twiddle and dividing by N)

template<std::size_t N>
void iterativeRadix2IDIT(std::array<std::complex<double>, N>& arr)
{
    bitReversalPermute(arr);

    // butterfly updates
    for (std::size_t len = 2; len <= N; len <<= 1)
    {
        // successive factors are powers of wlen = e^{-j·2π/len}
        std::complex<double> wlen = std::polar(1.0, 2.0 * M_PI / static_cast<double>(len));

        // loop through subarrays at stage log2(N) - log2(len)
        for (std::size_t i = 0; i < N; i += len)
        {
            std::complex<double> w(1.0, 0.0);

            for (std::size_t j = 0; j < len / 2; ++j)
            {
                std::complex<double> u = arr[i + j];               // even half
                std::complex<double> t = arr[i + j + len / 2] * w; // odd half

                arr[i + j]           = u + t;
                arr[i + j + len / 2] = u - t;

                w *= wlen;
            }
        }
    }

    for (auto& x : arr){
        x /= N;
    }
}

