#include "P_AudioConverter.hpp"

#include <iostream>

#include "P_Translation.hpp"

P_AudioConverter::P_AudioConverter():
    CTX(nullptr){
    std::cout << "[AC] Audio Converter Created" << std::endl;
}

P_AudioConverter::~P_AudioConverter(){
    std::cout << "[AC] Audio Converter Destructed" << std::endl;
    if(CTX != nullptr){
        swr_free(&CTX);
    }
}

int P_AudioConverter::Init(AVSampleFormat _in_fmt, int _in_rate, int _in_channels,
        AVSampleFormat _out_fmt, int _out_rate, int _out_channels){
    in_fmt = _in_fmt;
    in_rate = _in_rate;
    in_channels = _in_channels;
    out_fmt = _out_fmt;
    out_rate = _out_rate;
    out_channels = _out_channels;

    CTX = swr_alloc();
    if(CTX == nullptr){
        return -1;
    }

    auto CTXtmp = swr_alloc_set_opts(CTX, out_channels, out_fmt, out_rate,
        in_channels, in_fmt, in_rate, 0, nullptr);
    if(CTXtmp == nullptr){
        return -2;
    }

    auto status = swr_init(CTX);
    if(status < 0){
        return -3;
    }
    return 0;
}

int P_AudioConverter::Convert(const uint8_t **in, int in_len, uint8_t **out, int out_len){
    int status;
    return swr_convert(CTX, out, out_len, in, in_len);
}

SDL_AudioFormat P_AudioConverter::CompatibileFormat(AVSampleFormat fmt){
    switch(fmt){
        case AV_SAMPLE_FMT_U8P: return AUDIO_U8;
        case AV_SAMPLE_FMT_S16P: return AUDIO_S16;
        case AV_SAMPLE_FMT_S32P: return AUDIO_S32;

        case AV_SAMPLE_FMT_S64: return AUDIO_S32;
        case AV_SAMPLE_FMT_S64P: return AUDIO_S32;

        case AV_SAMPLE_FMT_DBL: return AUDIO_F32;
        case AV_SAMPLE_FMT_FLTP: return AUDIO_F32;
        case AV_SAMPLE_FMT_DBLP: return AUDIO_F32;
    }
    return AUDIO_F32;
}

int P_AudioConverter::GetBufferSize(int nb){
    return av_samples_get_buffer_size(
        nullptr, out_channels, nb, out_fmt, 1
    );
}