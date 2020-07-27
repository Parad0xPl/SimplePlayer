#pragma once

#include <cstdint>

#include <SDL2/SDL.h>

#include "libav.h"
#include "P_Frame.hpp"

class P_ImageConverter {
    private:
    int isInitilized;

    SwsContext *CTX;

    uint8_t *dst_image[4];
    int dst_linesizes[4];

    
    int src_width;
    int src_height; 
    int dst_width;
    int dst_height; 

    AVPixelFormat src_fmt;
    AVPixelFormat dst_fmt;

    public:
    P_ImageConverter();
    ~P_ImageConverter();

    int IsInited();

    int Init(
        int src_width, int src_height, AVPixelFormat src_fmt,
        int dst_width, int dst_height, AVPixelFormat dst_fmt
    );

    int Convert(P_Frame &frame);
    void CopyTo(void *data, int pitch, int height, SDL_PixelFormatEnum fmt);

    static AVPixelFormat BestFallback(AVPixelFormat src);
};