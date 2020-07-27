#pragma once

#include <cstdint>

#include "libav.h"

/**
 * @brief Wrapper around AVFrame structure
 * 
 */
class P_Frame{
    private:
    AVFrame *frame;

    public:
    P_Frame(AVFrame *ptr);
    P_Frame();
    ~P_Frame();

    void reset();

    AVFrame* ptr();
    uint8_t **data();
    int* linesize();
    int nb_samples();
};