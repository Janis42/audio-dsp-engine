#include <dr_wav.h>
#include <string>
#include <iostream>
#include <wav_data.h>

using std::cout;

WavData::WavData(){}

WavData::WavData(WavData&& wavobj){
    m_pSampleData = wavobj.m_pSampleData;
    wavobj.m_pSampleData = nullptr;
    m_format = wavobj.m_format;
    m_totalPCMFrameCount = wavobj.m_totalPCMFrameCount;
}

WavData& WavData::operator=(WavData&& wavobj){
    if (&wavobj == this) return *this;
    drwav_free(m_pSampleData, NULL);
    m_pSampleData = wavobj.m_pSampleData;
    wavobj.m_pSampleData = nullptr;
    m_format = wavobj.m_format;
    m_totalPCMFrameCount = wavobj.m_totalPCMFrameCount;
    return *this;
}

WavData::~WavData(){
    drwav_free(m_pSampleData, NULL);
}

bool WavData::saveWav(const char* path){
    drwav writer;

    if (!drwav_init_file_write(&writer, path, &m_format, NULL))
    {
        return false;
    }

    drwav_write_pcm_frames(&writer, m_totalPCMFrameCount, m_pSampleData);

    drwav_uninit(&writer);

    return true;
}

WavData::WavData(const char* path) {
    unsigned int channels;
    unsigned int sampleRate;
    m_pSampleData = drwav_open_file_and_read_pcm_frames_f32(
        path, // earlier "../soundfiles/wavTest.wav",
        &channels,
        &sampleRate,
        &m_totalPCMFrameCount,
        NULL);
    
    if (m_pSampleData == NULL) {
        throw std::runtime_error("File not found.");
    }
    
    m_format.container = drwav_container_riff;
    m_format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    m_format.channels = channels;
    m_format.sampleRate = sampleRate;
    m_format.bitsPerSample = 32;
}