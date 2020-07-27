#include "P_Translation.hpp"


namespace P_Translation{
    SDL_PixelFormatEnum PixAVFormat(AVPixelFormat fmt){
        #include "generated/PF_AVtoSDL.gensw"
        return SDL_PIXELFORMAT_UNKNOWN;
    }
    AVPixelFormat PixSDLFormat(SDL_PixelFormatEnum fmt){
        #include "generated/PF_SDLtoAV.gensw"
        return AV_PIX_FMT_NONE;
    }

    SDL_AudioFormat AudioAVFormat(AVSampleFormat fmt){
        #include "generated/SF_AVtoSDL.gensw"
        return 0;
    }
    AVSampleFormat AudioSDLFormat(SDL_AudioFormat fmt){
        #include "generated/SF_SDLtoAV.gensw"
        return AV_SAMPLE_FMT_NONE;
    }
}