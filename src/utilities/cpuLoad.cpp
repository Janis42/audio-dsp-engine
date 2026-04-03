#include <iostream>
#include <timer.h>
#include <input.h>

using std::cout;

void heatCPU(){
    // irrerelevant to main functionalities
    // function to monitor cpu temperature under computational load
    // current version operates on a single thread
    // function executes logistic map iteratively
    Timer t {};
    double x {0.314}; // arbitrary starting value
    double r {3.81}; // growth factor r set to chaos value
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