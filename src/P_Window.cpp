#include "P_Window.hpp"

#include <iostream>

P_Window::P_Window(int _w, int _h, const char* name, SDL_PixelFormatEnum fmt):
    width(_w),
    height(_h),
    winName(name),
    
    win(nullptr),
    renderer(nullptr),
    texture(nullptr),
    format(fmt){
    
}

P_Window::~P_Window(){
    if(renderer != nullptr){
        SDL_DestroyRenderer(renderer);
    }
    if(win != nullptr){
        SDL_DestroyWindow(win);
    }
    if(texture != nullptr){
        SDL_DestroyTexture(texture);
    }
}

int P_Window::Create(){
    win = SDL_CreateWindow(
        winName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_SHOWN
    );
    if(win == nullptr){
        std::cout << "[PW][ERR] Couldn't create window" << std::endl;
        return -1;
    }
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == nullptr){
        std::cout << "[PW][ERR] Couldn't create renderer" << std::endl;
        return -2;
    }
    std::cout << "[PW] Texture format: " << SDL_GetPixelFormatName(format) << std::endl;
    texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, width, height);
    if(texture == nullptr){
        std::cout << "[PW][ERR] Couldn't create texture" << std::endl;
        return -3;
    }
    return 0;
}

int P_Window::Lock(void **data, int *pitch){
    int status;

    status = SDL_LockTexture(texture, nullptr, data, pitch);
    if(status != 0){
        std::cout << "[PW][ERR] Couldn't lock texture" << std::endl;
        return -1;
    }
    return 0;
}

int P_Window::UnlockAndRender(){
    int status;

    SDL_UnlockTexture(texture);
    status = SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    if(status != 0){
        std::cout << "[PW][ERR] Couldn't render texture" << std::endl;
        return -1;
    }
    SDL_RenderPresent(renderer);
    return 0;
}