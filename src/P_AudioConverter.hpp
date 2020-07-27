#pragma once

#include <cstdint>

#include <SDL2/SDL.h>

#include "libav.h"

class P_AudioConverter {
    private:
    SwrContext *CTX;
    
    AVSampleFormat in_fmt;
    int in_rate;
    int in_channels;

    AVSampleFormat out_fmt;
    int out_rate;
    int out_channels;

    public:
    P_AudioConverter();
    ~P_AudioConverter();

    int Init(AVSampleFormat _in_fmt, int _in_rate, int _in_channels,
        AVSampleFormat _out_fmt, int _out_rate, int _out_channels);

    int Convert(const uint8_t **in, int in_len, uint8_t **out, int out_len);
    int GetBufferSize(int nb);

    static SDL_AudioFormat CompatibileFormat(AVSampleFormat fmt);
};