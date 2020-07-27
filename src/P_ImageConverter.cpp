#include "P_ImageConverter.hpp"

#include <iostream>
#include <cstdio>

P_ImageConverter::P_ImageConverter():
    isInitilized(0),
    CTX(nullptr),
    src_width(0),
    src_height(0),
    dst_width(0),
    dst_height(0),
    src_fmt(AV_PIX_FMT_NONE),
    dst_fmt(AV_PIX_FMT_NONE),
    
    dst_image{nullptr, nullptr, nullptr, nullptr},
    dst_linesizes{0,0,0,0}
    {
    std::cout << "[IC] Image Converter Created" << std::endl;
}

P_ImageConverter::~P_ImageConverter(){
    std::cout << "[IC] Image Converter Destructed" << std::endl;
    if(CTX != nullptr){
        sws_freeContext(CTX);
    }
    if(dst_image[0] != nullptr){
        av_freep(static_cast<void*>(&dst_image[0]));
    }
}

int P_ImageConverter::IsInited(){
    return isInitilized;
}

int P_ImageConverter::Init(
    int _src_width, int _src_height, AVPixelFormat _src_fmt,
    int _dst_width, int _dst_height, AVPixelFormat _dst_fmt
    ){
    if(isInitilized){
        return -1;
    }
    isInitilized = 1;
    if(_dst_fmt == AV_PIX_FMT_NONE){
        return -10;
    }

    src_width = _src_width;
    src_height = _src_height;
    src_fmt = _src_fmt;
    dst_width = _dst_width;
    dst_height = _dst_height;
    dst_fmt = _dst_fmt;

    int status;
    CTX = sws_getContext(src_width, src_height, src_fmt,
                         dst_width, dst_height, dst_fmt,
                         SWS_BILINEAR, nullptr, nullptr, nullptr);
    if(CTX == nullptr){
        std::cout << "[IC][ERR] Couldn't allocate context" << std::endl;
        return -1;
    }
    
    status = av_image_alloc(dst_image, dst_linesizes, dst_width, dst_height, dst_fmt, 1);
    if(status < 0){
        std::cout << "[IC][ERR] Couldn't allocate dst image buffers" << std::endl;
        return -2;
    }

    return 0;
}

int P_ImageConverter::Convert(P_Frame &frame){
    int status;

    // std::cout << "Frame Data: ";
    // PrintBuffer(frame.data()[0], 16);

    status = sws_scale(
        CTX, frame.data(), frame.linesize(),
        0, src_height, dst_image, dst_linesizes
    );
    if(status < 0){
        std::cout << "[IC][ERR] Couldn't convert frame" << std::endl;
        return -1;
    }
    // std::cout << "[" << status <<"] Converted Data: ";
    // PrintBuffer(dst_image[0], 16);

    return 0;
}

void P_ImageConverter::CopyTo(void *data, int pitch, int height, SDL_PixelFormatEnum fmt){
    // TODO: Change SDL_PixelFormatEnum to AVPixelFormat
    // TODO: Adjust buffer size calculation
    int size;
    const int sz_plane         = pitch * height;
    const int sz_plane_chroma  = ((pitch + 1) / 2) * ((height + 1) / 2);
    const int sz_plane_packed  = ((pitch + 1) / 2) * height;
    // Size calcucation based on SDL inside aprouch 
    switch(fmt) 
    {
        case SDL_PIXELFORMAT_YV12: /**< Planar mode: Y + V + U  (3 planes) */
        case SDL_PIXELFORMAT_IYUV: /**< Planar mode: Y + U + V  (3 planes) */
            size = sz_plane + sz_plane_chroma + sz_plane_chroma;
            break;

        case SDL_PIXELFORMAT_YUY2: /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
        case SDL_PIXELFORMAT_UYVY: /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
        case SDL_PIXELFORMAT_YVYU: /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
            size = 4 * sz_plane_packed;
            break;

        case SDL_PIXELFORMAT_NV12: /**< Planar mode: Y + U/V interleaved  (2 planes) */
        case SDL_PIXELFORMAT_NV21: /**< Planar mode: Y + V/U interleaved  (2 planes) */
            size = sz_plane + sz_plane_chroma + sz_plane_chroma;
            break;

        default:
            size = pitch * height;
            break;
    }
    // std::cout << "Size: " << size << std::endl;
    memcpy(data, dst_image[0], size);
}

AVPixelFormat P_ImageConverter::BestFallback(AVPixelFormat src){
    switch (src)
    {
        case AV_PIX_FMT_YUV420P: return AV_PIX_FMT_NV12; 
    }
    return AV_PIX_FMT_ARGB;
}