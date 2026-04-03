#ifndef WAV_DATA_H
#define WAV_DATA_H

#include <dr_wav.h>
#include <iostream>
using std::cout;

class WavData{
private:
    float* m_pSampleData;
    drwav_data_format m_format;
    drwav_uint64 m_totalPCMFrameCount;

public:
    WavData();
    WavData(const char* path);

    WavData(WavData&) = delete;
    WavData& operator=(WavData&) = delete;

    WavData(WavData&& wavobj);
    WavData& operator=(WavData&& wavobj);

    ~WavData();

    bool saveWav(const char* path);

    void printInfo(){
        cout << "WavData Object Info: [channels, sampleRate, totalPCMFrameCount] = [";
        cout << m_format.channels << ", " << m_format.sampleRate << ", " << m_totalPCMFrameCount << "]\n";
    }
    
};

#endif