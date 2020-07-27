#include "P_Frame.hpp"

#include <iostream>

P_Frame::P_Frame(AVFrame *ptr):
    frame(ptr){}

P_Frame::P_Frame(){
    frame = av_frame_alloc();
    if(frame == nullptr){
        std::cout << "[PV][ERR] Couldn't alloc frame" << std::endl;
        throw;
    }
}

P_Frame::~P_Frame(){
    av_frame_free(&frame);
}

AVFrame* P_Frame::ptr(){
    return frame;
}

void P_Frame::reset(){
    av_frame_free(&frame);
    frame = av_frame_alloc();
    if(frame == nullptr){
        std::cout << "[PV][ERR] Couldn't alloc frame" << std::endl;
        throw;
    }
}

uint8_t **P_Frame::data(){
    return frame->data;
}

int *P_Frame::linesize(){
    return frame->linesize;
}

int P_Frame::nb_samples(){
    return frame->nb_samples;
}