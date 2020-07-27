#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include "SDL2/SDL.h"

#include "libav.h"
#include "P_ImageConverter.hpp"
#include "P_Frame.hpp"


class P_VideoFile {
    private:
    std::string path;
    AVFormatContext *CTX;

    AVCodec *AudioCodec;
    int AudioStreamID;
    AVCodec *VideoCodec;
    int VideoStreamID;

    AVCodecContext *AudioCodecCTX;
    AVCodecContext *VideoCodecCTX;

    public:
    P_VideoFile(std::string &filepath);
    ~P_VideoFile();

    int OpenFile();
    int AllocCodecCTX();
    int DecodeFrame(AVMediaType &type, int& got_frame, P_Frame &frame);
    int FillAudioSpec(SDL_AudioSpec &spec);
    int GetAudioBufferSize(int nb_samples);

    void ProcessAudioFrame(P_Frame &frame);
    void ProcessVideoFrame(P_Frame &frame);

    int GetWidth();
    int GetHeight();

    AVPixelFormat GetPixelFormat();

    AVSampleFormat GetAudioFormat();
    uint64_t GetChannelsFormat();
};