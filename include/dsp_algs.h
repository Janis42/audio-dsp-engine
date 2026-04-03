#ifndef DSP_ALGS_H
#define DSP_ALGS_H

#include <array>
#include <cmath>
#include <stdexcept>
#include <string>

template<std::size_t num_vals>
void recursiveRadix2DIT(std::array<std::complex<double>, num_vals>& arr);

class RecursiveFilter {
public:
    virtual double process(double) = 0;
    virtual void reset() = 0;
    virtual ~RecursiveFilter() = default;
};

// implement application to signal into WavData class
// test out implementation by applying low pass with common frequency to signal and saving file

// define more filters derived from that class

class Biquad: public RecursiveFilter{
private:
    double b0{}, b1{}, b2{};
    double a1{}, a2{};
    double z1{}, z2{}; // latent state
public:
    // dBgain is only used for peakingEQ; ignored by all other modes
    Biquad(const std::string& mode, double f_s, double f_0, double Q, double dBgain = 0.0);
    
    double process(double x) override;
    
    void reset() override;
};

#endif