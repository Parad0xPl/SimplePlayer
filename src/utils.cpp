#include "utils.hpp"

#include <cstdio>
#include <cstdint>
#include <cstring>

void PrintBuffer(void *data, int len){
    const auto p = static_cast<uint8_t*>(data);
    int i;
    for (i = 0; i < len; i++)
    {
        if (i > 0) printf(":");
        printf("%02X", p[i]);
    }
    printf("\n");
}

void PrintBufferMax(void *data, int len1, int len2){
    int len = len1>len2 ? len2 : len1;
    PrintBuffer(data, len);
}

void PrintAudioFloat(void *data, int len){
    const auto p = static_cast<float*>(data);
    int i;
    for (i = 0; i < len; i++)
    {
        if (i > 0) printf(":");
        printf("%F", p[i]);
    }
    printf("\n");
}

void MixPlanarAudio(uint8_t** src, uint8_t* dst, 
    int sampleSize, int nb){
    int i;
    uint8_t *pt = dst;
    for(i=0;i<nb;i++){
        memcpy(pt, src[0]+i*sampleSize, sampleSize);
        pt+=sampleSize;
        memcpy(pt, src[1]+i*sampleSize, sampleSize);
        pt+=sampleSize;
    }
    // for(i=0;i<linesizes[0];i++){
    //     ((uint32_t*) dst)[i] = ((uint32_t*) src[0])[i];
    //     // ((uint32_t*) dst)[i << 1] = ((uint32_t*) src[0])[i];
    // }
}