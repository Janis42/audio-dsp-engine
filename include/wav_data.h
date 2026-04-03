#ifndef WAV_DATA_H
#define WAV_DATA_H

#include <dr_wav.h>
#include <iostream>
using std::cout;

class RecursiveFilter;

class WavData{
private:
    double* m_pSampleData;
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

    drwav_data_format getFormat();

    bool saveWav(const char* path);

    void printInfo();

    void applyFilter(RecursiveFilter& filter);
};

#endif