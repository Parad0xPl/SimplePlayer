#pragma once

#include "SDL2/SDL.h"

class P_Window{
    private:
    int width;
    int height;
    const char* winName;

    SDL_Window* win;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_PixelFormatEnum format;

    public:
    P_Window(int _w, int _h, const char* name, SDL_PixelFormatEnum fmt);
    ~P_Window();

    int Create();

    int Lock(void **data, int *pitch);
    int UnlockAndRender();
};