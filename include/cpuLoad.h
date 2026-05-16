#ifndef CPULOAD_H
#define CPULOAD_H

void heatCPU();


void superScalarTest(std::size_t num, int evals);
void threadedTest(int nthreads, int evals);
void combinedTest(std::size_t num, int nthreads, int evals);

#endif