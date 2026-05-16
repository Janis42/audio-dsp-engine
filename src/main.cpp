#include "dr_wav.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <timer.h>
#include <dsp_algs.h>
#include <wav_data.h>
#include <cpuLoad.h>

using std::cout;

int main(int argc, char* argv[]){

    std::string mode{(argc>1)?argv[1]:"default"};

    if (mode=="playground"){
        // some dummy code to test functionalities
        try {
            WavData dataobj {"soundfiles/wavTest.wav"};
            cout << "File successfully loaded\n";
            dataobj.printInfo();

            Biquad filter("lpf", dataobj.getFormat().sampleRate, 200.0, 0.707);
            dataobj.applyFilter(filter);
            // saving the modified file
            dataobj.saveWav("soundfiles/output_lowpass2.wav");
        }

        catch (const std::runtime_error& err) {
            cout << err.what() << std::endl;
            return 1;
        }
    }

    if (mode=="time_algs"){
        Timer timer;
        
        timer.reset();
        return 0;
    }
    
    // only here for some preliminary tests
    if (mode=="heatcpu"){
        Timer t {};

        for (std::size_t s : {1, 2, 4, 8, 16, 32, 64, 128}){
            //t.reset();
            //superScalarTest(s, 10'000'000);
            //cout << "\nsuperScalar s = " << s << ": " << t.elapsed() << std::endl;

            t.reset();
            threadedTest(s, 1'000'000'000);
            cout << "\nthreaded   s = " << s << ": " << t.elapsed() << std::endl;

            t.reset();
            combinedTest(s, std::min(s, std::size_t{16}), 1'000'000'000);
            cout << "\ncombined   s = " << s << ": " << t.elapsed() << std::endl;
        }
        
        
        cout << "Started heat CPU mode, 0 stops the program. \n";
        heatCPU();
    }
    
    return 0;
}