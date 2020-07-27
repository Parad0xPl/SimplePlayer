#pragma once

#include <cstdint>

void PrintBuffer(void *data, int len);
void PrintBufferMax(void *data, int len1, int len2);

void PrintAudioFloat(void *data, int len);

void MixPlanarAudio(uint8_t** src, uint8_t* dst, 
    int sampleSize, int nb);