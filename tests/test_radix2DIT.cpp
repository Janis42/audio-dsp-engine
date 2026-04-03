// =============================================================================
// test_radix2DIT.cpp — Unit tests for the radix-2 DIT FFT implementation
// =============================================================================
//
// Tests the recursiveRadix2DIT<double, N>() function using known analytical
// results from DFT theory.  Each test targets a specific property of the
// Discrete Fourier Transform so that failures pinpoint *what* is broken.
//
// Convention used throughout:
//   X[k] = Σ_{n=0}^{N-1}  x[n] · e^{-j·2π·k·n / N}      (forward DFT)
//
// Google Test macros used:
//   TEST(SuiteName, TestName)   — defines an independent test case
//   EXPECT_NEAR(a, b, tol)     — passes if |a - b| <= tol  (soft fail)
//   ASSERT_NEAR(a, b, tol)     — same but aborts the test on failure
// =============================================================================

#include <gtest/gtest.h>

// The implementation is header-only (all templates), so we include the .cpp
// directly.  In a mature project this would be a .hpp in include/.
#include "audio_processing/radix2DIT.cpp"

#include <array>
#include <cmath>
#include <complex>

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

constexpr double kTol = 1e-9; // tolerance

using Cd = std::complex<double>;

// Google Test's EXPECT_NEAR only works on scalars -> separate real and imaginary part
#define EXPECT_COMPLEX_NEAR(actual, expected, tol)                           \
    EXPECT_NEAR((actual).real(), (expected).real(), tol)                      \
        << "real part mismatch";                                             \
    EXPECT_NEAR((actual).imag(), (expected).imag(), tol)                     \
        << "imaginary part mismatch"

// DFT of a length-1 signal is the signal itself: X[0] = x[0]
TEST(Radix2DIT, SingleElement_IsIdentity)
{
    std::array<Cd, 1> signal = {Cd(3.7, -1.2)};
    const Cd original = signal[0]; // save a copy before the in-place transform

    recursiveRadix2DIT(signal);

    EXPECT_COMPLEX_NEAR(signal[0], original, kTol);
}

// 2. Length-2 — smallest butterfly
// For x = {a, b}:
//   X[0] = a + b        (sum)
//   X[1] = a - b        (difference, since W_2^0 = 1)
TEST(Radix2DIT, TwoPoint_SumAndDifference)
{
    std::array<Cd, 2> signal = {Cd(1.0, 0.0), Cd(2.0, 0.0)};

    recursiveRadix2DIT(signal);

    EXPECT_COMPLEX_NEAR(signal[0], Cd(3.0, 0.0), kTol); // 1 + 2
    EXPECT_COMPLEX_NEAR(signal[1], Cd(-1.0, 0.0), kTol); // 1 - 2
}

// 3. DC signal (constant) — all energy in bin 0

// If every sample equals C, the DFT is:
//   X[0] = N·C,   X[k] = 0  for k >= 1
// Using N = 4, C = 1
TEST(Radix2DIT, DCSignal_AllEnergyInBinZero)
{
    constexpr std::size_t N = 4;
    std::array<Cd, N> signal = {Cd(1.0), Cd(1.0), Cd(1.0), Cd(1.0)};

    recursiveRadix2DIT(signal);

    // Bin 0 should hold N * 1.0 = 4.0
    EXPECT_COMPLEX_NEAR(signal[0], Cd(4.0, 0.0), kTol);

    // All other bins should be zero
    for (std::size_t k = 1; k < N; ++k)
    {
        EXPECT_COMPLEX_NEAR(signal[k], Cd(0.0, 0.0), kTol)
            << "Non-zero energy in bin " << k;
    }
}

// 4. Pure cosine at bin 1 — single frequency detection

// x[n] = cos(2π·n/N) = ½·e^{j2πn/N} + ½·e^{-j2πn/N}
// So: X[1] = N/2,  X[N-1] = N/2,  all others = 0

TEST(Radix2DIT, PureCosine_PeaksAtBin1AndBinNminus1)
{
    constexpr std::size_t N = 8;
    std::array<Cd, N> signal{};

    for (std::size_t n = 0; n < N; ++n)
    {
        signal[n] = Cd(std::cos(2.0 * M_PI * n / N), 0.0);
    }

    recursiveRadix2DIT(signal);

    // Bins 1 and N-1 should hold N/2 = 4.0
    EXPECT_COMPLEX_NEAR(signal[1], Cd(N / 2.0, 0.0), kTol);
    EXPECT_COMPLEX_NEAR(signal[N - 1], Cd(N / 2.0, 0.0), kTol);

    // Every other bin should be zero
    for (std::size_t k = 0; k < N; ++k)
    {
        if (k == 1 || k == N - 1) continue;
        EXPECT_NEAR(std::abs(signal[k]), 0.0, kTol)
            << "Unexpected energy in bin " << k;
    }
}

// 5. Impulse — DFT is flat (all bins = 1)
// x = {1, 0, 0, …, 0}  →  X[k] = 1 for all k

TEST(Radix2DIT, Impulse_FlatSpectrum)
{
    constexpr std::size_t N = 8;
    std::array<Cd, N> signal{};
    signal[0] = Cd(1.0, 0.0); // all others are 0 by value-initialization

    recursiveRadix2DIT(signal);

    for (std::size_t k = 0; k < N; ++k)
    {
        EXPECT_COMPLEX_NEAR(signal[k], Cd(1.0, 0.0), kTol)
            << "Bin " << k << " should be 1.0 for an impulse input";
    }
}

// 6. Parseval's theorem — energy conservation
// Σ|x[n]|² = (1/N) · Σ|X[k]|²
// We feed an arbitrary real signal and check that time-domain energy equals
// frequency-domain energy (scaled).  This catches magnitude-mangling bugs
// even when the phase pattern happens to look correct.

TEST(Radix2DIT, Parseval_EnergyIsConserved)
{
    constexpr std::size_t N = 8;
    std::array<Cd, N> signal = {
        Cd(0.1), Cd(-0.4), Cd(0.9), Cd(0.3),
        Cd(-0.7), Cd(0.5), Cd(0.2), Cd(-0.8)};

    // Compute time-domain energy before the in-place transform destroys x[n]
    double timeDomainEnergy = 0.0;
    for (const auto& s : signal)
    {
        timeDomainEnergy += std::norm(s); // norm = |z|²
    }

    recursiveRadix2DIT(signal);

    // Compute frequency-domain energy
    double freqDomainEnergy = 0.0;
    for (const auto& S : signal)
    {
        freqDomainEnergy += std::norm(S);
    }
    freqDomainEnergy /= N; // Parseval scaling factor

    EXPECT_NEAR(timeDomainEnergy, freqDomainEnergy, kTol);
}

// 7. Linearity:  DFT(a·x + b·y) == a·DFT(x) + b·DFT(y)

TEST(Radix2DIT, Linearity_SuperpositionHolds)
{
    constexpr std::size_t N = 4;
    const Cd a(2.0, 0.0);
    const Cd b(-1.5, 0.0);

    // Two arbitrary input signals
    std::array<Cd, N> x = {Cd(1.0), Cd(0.0), Cd(-1.0), Cd(0.5)};
    std::array<Cd, N> y = {Cd(0.3), Cd(-0.7), Cd(0.4), Cd(1.0)};

    // Build the combined signal:  z[n] = a·x[n] + b·y[n]
    std::array<Cd, N> z{};
    for (std::size_t n = 0; n < N; ++n)
    {
        z[n] = a * x[n] + b * y[n];
    }

    // Transform all three independently
    recursiveRadix2DIT(x);
    recursiveRadix2DIT(y);
    recursiveRadix2DIT(z);

    // Z[k] should equal a·X[k] + b·Y[k] for every bin
    for (std::size_t k = 0; k < N; ++k)
    {
        Cd expected = a * x[k] + b * y[k];
        EXPECT_COMPLEX_NEAR(z[k], expected, kTol)
            << "Linearity violated at bin " << k;
    }
}

// 8. Known 4-point DFT — hand-verified numerical result
// x = {1, 2, 3, 4}
// By direct computation:
//   X[0] = 10 + 0j
//   X[1] = -2 + 2j
//   X[2] = -2 + 0j
//   X[3] = -2 - 2j
TEST(Radix2DIT, KnownFourPoint_MatchesHandComputation)
{
    constexpr std::size_t N = 4;
    std::array<Cd, N> signal = {Cd(1.0), Cd(2.0), Cd(3.0), Cd(4.0)};

    recursiveRadix2DIT(signal);

    EXPECT_COMPLEX_NEAR(signal[0], Cd(10.0, 0.0), kTol);
    EXPECT_COMPLEX_NEAR(signal[1], Cd(-2.0, 2.0), kTol);
    EXPECT_COMPLEX_NEAR(signal[2], Cd(-2.0, 0.0), kTol);
    EXPECT_COMPLEX_NEAR(signal[3], Cd(-2.0, -2.0), kTol);
}

// =============================================================================
// Test Suite: IterativeRadix2DIT — In-place version correctness
// =============================================================================

// Test 1: Match recursive on known small input
TEST(IterativeRadix2DIT, SmallInput_MatchesRecursive)
{
    constexpr std::size_t N = 8;
    std::array<Cd, N> recursive_input = {
        Cd(1.0), Cd(-0.5), Cd(0.2), Cd(0.8),
        Cd(-0.3), Cd(0.7), Cd(0.1), Cd(-0.4)};
    std::array<Cd, N> iterative_input = recursive_input;

    // Run both versions
    recursiveRadix2DIT(recursive_input);
    iterativeRadix2DIT(iterative_input);

    // Compare bin-by-bin
    for (std::size_t k = 0; k < N; ++k)
    {
        EXPECT_COMPLEX_NEAR(iterative_input[k], recursive_input[k], kTol)
            << "Mismatch at bin " << k;
    }
}

// Test 2: Iterative version on larger size (typical audio N=4096)
// Use Parseval's theorem to verify without comparing to recursive
TEST(IterativeRadix2DIT, LargeInputParseval_N4096)
{
    constexpr std::size_t N = 4096;
    std::array<Cd, N> signal{};

    // Generate pseudorandom signal (using simple deterministic pattern for reproducibility)
    for (std::size_t i = 0; i < N; ++i)
    {
        double val = std::sin(2.0 * M_PI * i / N) * 0.5 + std::cos(4.0 * M_PI * i / N) * 0.3;
        signal[i] = Cd(val, 0.0);
    }

    // Compute time-domain energy
    double timeDomainEnergy = 0.0;
    for (const auto& s : signal)
    {
        timeDomainEnergy += std::norm(s);
    }

    // Run iterative FFT
    iterativeRadix2DIT(signal);

    // Compute frequency-domain energy
    double freqDomainEnergy = 0.0;
    for (const auto& S : signal)
    {
        freqDomainEnergy += std::norm(S);
    }
    freqDomainEnergy /= N;

    // Parseval: time-domain energy == frequency-domain energy
    EXPECT_NEAR(timeDomainEnergy, freqDomainEnergy, kTol);
}

// Test 3: Impulse on iterative (should give flat spectrum)
TEST(IterativeRadix2DIT, Impulse_FlatSpectrum)
{
    constexpr std::size_t N = 16;
    std::array<Cd, N> signal{};
    signal[0] = Cd(1.0, 0.0);

    iterativeRadix2DIT(signal);

    for (std::size_t k = 0; k < N; ++k)
    {
        EXPECT_COMPLEX_NEAR(signal[k], Cd(1.0, 0.0), kTol)
            << "Bin " << k << " should be 1.0 for impulse";
    }
}

// =============================================================================
// Test Suite: IterativeRadix2IDIT — Inverse FFT correctness
// =============================================================================

// Test 1: Round-trip — FFT then IFFT recovers the original signal
// IFFT(FFT(x)) == x  (up to floating-point error)
TEST(IterativeRadix2IDIT, RoundTrip_RecoverOriginalSignal)
{
    constexpr std::size_t N = 8;
    std::array<Cd, N> original = {
        Cd(1.0), Cd(-0.5), Cd(0.2), Cd(0.8),
        Cd(-0.3), Cd(0.7), Cd(0.1), Cd(-0.4)};
    std::array<Cd, N> signal = original;

    iterativeRadix2DIT(signal);
    iterativeRadix2IDIT(signal);

    for (std::size_t n = 0; n < N; ++n)
    {
        EXPECT_COMPLEX_NEAR(signal[n], original[n], kTol)
            << "Round-trip mismatch at sample " << n;
    }
}

// Test 2: IFFT of a flat (DC) spectrum gives an impulse at n=0
// X[k] = 1 for all k  =>  x[n] = δ[n]  (impulse at 0)
TEST(IterativeRadix2IDIT, FlatSpectrum_GivesImpulse)
{
    constexpr std::size_t N = 8;
    std::array<Cd, N> spectrum;
    spectrum.fill(Cd(1.0, 0.0));

    iterativeRadix2IDIT(spectrum);

    EXPECT_COMPLEX_NEAR(spectrum[0], Cd(1.0, 0.0), kTol);
    for (std::size_t n = 1; n < N; ++n)
    {
        EXPECT_COMPLEX_NEAR(spectrum[n], Cd(0.0, 0.0), kTol)
            << "Non-zero at sample " << n << " for flat spectrum IFFT";
    }
}

// Test 3: Known 4-point IFFT — hand-verified result
// X = {10, -2+2j, -2, -2-2j}  =>  x = {1, 2, 3, 4}
TEST(IterativeRadix2IDIT, KnownFourPoint_MatchesHandComputation)
{
    constexpr std::size_t N = 4;
    std::array<Cd, N> spectrum = {Cd(10.0, 0.0), Cd(-2.0, 2.0), Cd(-2.0, 0.0), Cd(-2.0, -2.0)};

    iterativeRadix2IDIT(spectrum);

    EXPECT_COMPLEX_NEAR(spectrum[0], Cd(1.0, 0.0), kTol);
    EXPECT_COMPLEX_NEAR(spectrum[1], Cd(2.0, 0.0), kTol);
    EXPECT_COMPLEX_NEAR(spectrum[2], Cd(3.0, 0.0), kTol);
    EXPECT_COMPLEX_NEAR(spectrum[3], Cd(4.0, 0.0), kTol);
}

// Test 4: Single bin set — IFFT gives a complex sinusoid
// X[k0] = N, all others 0  =>  x[n] = e^{j·2π·k0·n/N}
TEST(IterativeRadix2IDIT, SingleBin_GivesComplexSinusoid)
{
    constexpr std::size_t N = 8;
    constexpr std::size_t k0 = 2;
    std::array<Cd, N> spectrum{};
    spectrum[k0] = Cd(static_cast<double>(N), 0.0);

    iterativeRadix2IDIT(spectrum);

    for (std::size_t n = 0; n < N; ++n)
    {
        Cd expected = std::polar(1.0, 2.0 * M_PI * k0 * n / N);
        EXPECT_COMPLEX_NEAR(spectrum[n], expected, kTol)
            << "Sinusoid mismatch at sample " << n;
    }
}

// Test 5: Round-trip at larger size (N=1024)
TEST(IterativeRadix2IDIT, RoundTrip_N1024)
{
    constexpr std::size_t N = 1024;
    std::array<Cd, N> original{};
    for (std::size_t i = 0; i < N; ++i)
    {
        original[i] = Cd(std::sin(2.0 * M_PI * 5 * i / N), std::cos(2.0 * M_PI * 3 * i / N));
    }
    std::array<Cd, N> signal = original;

    iterativeRadix2DIT(signal);
    iterativeRadix2IDIT(signal);

    for (std::size_t n = 0; n < N; ++n)
    {
        EXPECT_COMPLEX_NEAR(signal[n], original[n], 1e-9)
            << "Round-trip mismatch at sample " << n;
    }
}

// Test 4: DC signal on iterative (all energy in bin 0)
TEST(IterativeRadix2DIT, DCSignal_AllEnergyInBinZero)
{
    constexpr std::size_t N = 8;
    std::array<Cd, N> signal = {
        Cd(1.0), Cd(1.0), Cd(1.0), Cd(1.0),
        Cd(1.0), Cd(1.0), Cd(1.0), Cd(1.0)};

    iterativeRadix2DIT(signal);

    EXPECT_COMPLEX_NEAR(signal[0], Cd(8.0, 0.0), kTol);

    for (std::size_t k = 1; k < N; ++k)
    {
        EXPECT_COMPLEX_NEAR(signal[k], Cd(0.0, 0.0), kTol)
            << "Non-zero energy in bin " << k;
    }
}
