#include "dr_wav.h"
#include <iostream>
#include <string>
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
        cout << "Started heat CPU mode, 0 stops the program. \n";
        heatCPU();
    }
    
    return 0;
}