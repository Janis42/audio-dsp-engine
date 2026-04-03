#include <dr_wav.h>
#include <string>
#include <iostream>
#include <wav_data.h>
#include <dsp_algs.h>

using std::cout;

WavData::WavData(){}

WavData::WavData(const char* path) {
    unsigned int channels;
    unsigned int sampleRate;
    float* buffer = drwav_open_file_and_read_pcm_frames_f32(
        path, // earlier "../soundfiles/wavTest.wav",
        &channels,
        &sampleRate,
        &m_totalPCMFrameCount,
        NULL);

    if (buffer == NULL) {
        throw std::runtime_error("File not found.");
    }

    m_format.container = drwav_container_riff;
    m_format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    m_format.channels = channels;
    m_format.sampleRate = sampleRate;
    m_format.bitsPerSample = 32;

    // convert float → double for internal processing
    const drwav_uint64 totalSamples = m_totalPCMFrameCount * channels;
    m_pSampleData = new double[totalSamples];
    for (drwav_uint64 i = 0; i < totalSamples; ++i) {
        m_pSampleData[i] = static_cast<double>(buffer[i]);
    }
    drwav_free(buffer, NULL);
}

WavData::WavData(WavData&& wavobj){
    m_pSampleData = wavobj.m_pSampleData;
    wavobj.m_pSampleData = nullptr;
    m_format = wavobj.m_format;
    m_totalPCMFrameCount = wavobj.m_totalPCMFrameCount;
}

WavData& WavData::operator=(WavData&& wavobj){
    if (&wavobj == this) return *this;
    delete[] m_pSampleData;
    m_pSampleData = wavobj.m_pSampleData;
    wavobj.m_pSampleData = nullptr;
    m_format = wavobj.m_format;
    m_totalPCMFrameCount = wavobj.m_totalPCMFrameCount;
    return *this;
}

WavData::~WavData(){
    delete[] m_pSampleData;
}

drwav_data_format WavData::getFormat(){
    return m_format;
}

bool WavData::saveWav(const char* path){
    drwav writer;

    if (!drwav_init_file_write(&writer, path, &m_format, NULL))
    {
        return false;
    }

    // convert double → float for 32-bit WAV output
    const drwav_uint64 totalSamples = m_totalPCMFrameCount * m_format.channels;
    float* floatBuf = new float[totalSamples];
    for (drwav_uint64 i = 0; i < totalSamples; ++i) {
        floatBuf[i] = static_cast<float>(m_pSampleData[i]);
    }

    drwav_write_pcm_frames(&writer, m_totalPCMFrameCount, floatBuf);

    delete[] floatBuf;
    drwav_uninit(&writer);

    return true;
}

void WavData::printInfo() {
        cout << "WavData Object Info: [channels, sampleRate, totalPCMFrameCount] = [";
        cout << m_format.channels << ", " << m_format.sampleRate << ", " << m_totalPCMFrameCount << "]\n";
    }

void WavData::applyFilter(RecursiveFilter& filter) {
    int channels {m_format.channels};
    for (int c {0}; c<channels; c++){
        filter.reset();
        for (int i {0}; i<m_totalPCMFrameCount; i++){
            // indexing starts at 1 to simplify computation
            int ind {c+channels*i};
            m_pSampleData[ind] = filter.process(m_pSampleData[ind]);
        }
    }
}
