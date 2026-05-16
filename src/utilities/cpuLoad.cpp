#include <iostream>
#include <vector>
#include <omp.h>
#include <timer.h>
#include <input.h>
#include <cmath>

using std::cout;

constexpr double R_CHAOS = 3.81;
constexpr double PI_START = M_PI/10;

template<const double r>
double logisticMap(double x){
    return r*x*(1-x);
}

void heatCPU(){
    // irrerelevant to main functionalities
    // function to monitor cpu temperature under computational load
    // current version operates on a single thread
    // function executes logistic map iteratively
    Timer t {};
    double x {0.314}; // arbitrary starting value
    double r {R_CHAOS}; // growth factor r set to chaos value
    long long billion_iterations;
    while(true){
        billion_iterations = getVal<long long>();
        if (billion_iterations==0) break;
        cout << "Running for " << billion_iterations << " billion iterations.\n";
        t.reset();
        for (long long i {0}; i<1'000'000'000LL * billion_iterations; i++){
            x = r*x*(1-x); // logistic map
        }
        cout << "Final value:" << x << "\n";
        cout << "Elapsed time: " << t.elapsed() << "s\n";
    }
}


void superScalarTest(const std::size_t num, int evals){
    double vals[num];
    // initialize array with different non-zero values
    for (std::size_t i{0}; i<num; i++){
        vals[i] = PI_START*(i+1)/num;
    }
    for (int i = 0; i<evals; i++){
        for (std::size_t j = 0; j<num; j++){
            vals[j] = logisticMap<R_CHAOS>(vals[j]);
        }
    }
    double sum {0};
    for (std::size_t i = 0; i < num; i++){
        sum+=vals[i];
    }
    cout << sum;
}


void threadedTest(int nthreads, int evals){
    // each thread runs one independent logistic map chain
    // use local variable to avoid false sharing, copy back at the end
    std::vector<double> vals(nthreads);
    for (int i = 0; i < nthreads; i++){
        vals[i] = PI_START * (i + 1) / nthreads;
    }
    omp_set_num_threads(nthreads);
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        double local = vals[tid];
        for (int i = 0; i < evals; i++){
            local = logisticMap<R_CHAOS>(local);
        }
        vals[tid] = local;
    }
    double sum {0};
    for (std::size_t i = 0; i < nthreads; i++){
        sum+=vals[i];
    }
    cout << sum;
}


template<std::size_t N>
static void runChains(double* data, int evals){
    double local[N];
    for (std::size_t j = 0; j < N; j++) local[j] = data[j];
    for (int i = 0; i < evals; i++){
        for (std::size_t j = 0; j < N; j++){
            local[j] = logisticMap<R_CHAOS>(local[j]);
        }
    }
    for (std::size_t j = 0; j < N; j++) data[j] = local[j];
}

void combinedTest(std::size_t num, int nthreads, int evals){
    std::vector<double> vals(num);
    for (std::size_t i = 0; i < num; i++){
        vals[i] = PI_START * (i + 1) / num;
    }
    std::size_t cpt = num / nthreads;
    omp_set_num_threads(nthreads);
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        double* start = &vals[tid * cpt];
        if (cpt == 1) runChains<1>(start, evals);
        else if (cpt == 2) runChains<2>(start, evals);
        else if (cpt == 4) runChains<4>(start, evals);
        else if (cpt == 8) runChains<8>(start, evals);
        else if (cpt == 16) runChains<16>(start, evals);
    }
    double sum {0};
    for (std::size_t i = 0; i < num; i++){
        sum+=vals[i];
    }
    cout << sum;
}