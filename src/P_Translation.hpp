#pragma once

#include <SDL2/SDL.h>

#include "libav.h"

namespace P_Translation{
    SDL_PixelFormatEnum PixAVFormat(AVPixelFormat fmt);
    AVPixelFormat PixSDLFormat(SDL_PixelFormatEnum fmt);

    SDL_AudioFormat AudioAVFormat(AVSampleFormat fmt);
    AVSampleFormat AudioSDLFormat(SDL_AudioFormat fmt);
}