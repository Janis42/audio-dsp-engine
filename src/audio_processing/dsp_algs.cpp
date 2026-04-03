#include <dsp_algs.h>

Biquad::Biquad(const std::string& mode, double f_s, double f_0, double Q, double dBgain)
: z1{0}
, z2{0}
{
    const double w0      = 2.0 * M_PI * f_0 / f_s;
    const double cos_w0  = std::cos(w0);
    const double sin_w0  = std::sin(w0);
    const double alpha   = sin_w0 / (2.0 * Q);

    double a0{};

    if (mode == "lpf") {
        b0 = (1.0 - cos_w0) / 2.0;
        b1 =  1.0 - cos_w0;
        b2 = (1.0 - cos_w0) / 2.0;
        a0 =  1.0 + alpha;
        a1 = -2.0 * cos_w0;
        a2 =  1.0 - alpha;
    } else if (mode == "hpf") {
        b0 =  (1.0 + cos_w0) / 2.0;
        b1 = -(1.0 + cos_w0);
        b2 =  (1.0 + cos_w0) / 2.0;
        a0 =   1.0 + alpha;
        a1 =  -2.0 * cos_w0;
        a2 =   1.0 - alpha;
    } else if (mode == "bpf") {
        // constant 0 dB peak gain variant
        b0 =  sin_w0 / 2.0;
        b1 =  0.0;
        b2 = -sin_w0 / 2.0;
        a0 =  1.0 + alpha;
        a1 = -2.0 * cos_w0;
        a2 =  1.0 - alpha;
    } else if (mode == "notch") {
        b0 =  1.0;
        b1 = -2.0 * cos_w0;
        b2 =  1.0;
        a0 =  1.0 + alpha;
        a1 = -2.0 * cos_w0;
        a2 =  1.0 - alpha;
    } else if (mode == "peakingEQ") {
        const double A = std::pow(10.0, dBgain / 40.0);
        b0 =  1.0 + alpha * A;
        b1 = -2.0 * cos_w0;
        b2 =  1.0 - alpha * A;
        a0 =  1.0 + alpha / A;
        a1 = -2.0 * cos_w0;
        a2 =  1.0 - alpha / A;
    } else {
        throw std::invalid_argument("Biquad: unknown mode '" + mode + "'");
    }

    // normalize by a0
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
}

double Biquad::process(double x){
    double y = b0 * x + z1;
    z1 = b1 * x - a1 * y + z2;
    z2 = b2 * x - a2 * y;
    return y;
}

void Biquad::reset(){
    z1 = z2 = 0.0;
}