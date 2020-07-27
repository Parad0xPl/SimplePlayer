#include <iostream>
#include <filesystem>
#include <memory>
#include <string>

#include <SDL2/SDL.h>

#include "utils.hpp"
#include "P_ImageConverter.hpp"
#include "P_AudioConverter.hpp"
#include "P_VideoFile.hpp"
#include "P_Window.hpp"
#include "P_Translation.hpp"

class MainCTX{
    private:
    std::unique_ptr<P_Window> window;
    std::unique_ptr<P_VideoFile> video;

    const char* AudioTarget;
    SDL_AudioDeviceID AudioTargetID;
    uint8_t *AudioBuffer;
    const int AudioBufferSize = 1024 * 64;

    P_ImageConverter ImageConverter;
    P_AudioConverter AudioConverter;
    int isAudioConverterUsed = 0;

    SDL_PixelFormatEnum SurfaceTargetFormat;

    std::string VideoFilePath;

    public:
    MainCTX(std::string &path, const char* AudioPreferedName):
        VideoFilePath(path),
        AudioBuffer(nullptr),
        SurfaceTargetFormat(SDL_PIXELFORMAT_UNKNOWN){
        video = std::make_unique<P_VideoFile>(VideoFilePath);

        const int AudioOutputNum = SDL_GetNumAudioDevices(0);
        AudioTarget = nullptr;
        if(AudioOutputNum > 0){
            AudioTarget = SDL_GetAudioDeviceName(0, 0);
        }
        int i;
        for(i=0;i<AudioOutputNum;i++){
            const auto name = SDL_GetAudioDeviceName(i, 0);
            std::cout << "[" << i << "] '" << name << "'" << std::endl;
            if(std::string(name).find(AudioPreferedName) != std::string::npos){
                std::cout << "[Audio] Output found: '" << name << "'" << std::endl;
                AudioTarget = name;
            }
            
        }
    }

    ~MainCTX(){
        if(AudioBuffer){
            free(AudioBuffer);
        }
        video.release();
    }

    int OpenAudio(){
        std::cout << "[Audio] Opening audio resources" << std::endl;
        if(AudioTarget < 0){
            std::cout << "[Audio][ERR] No audio output device" << std::endl;
            return -1;
        }

        SDL_AudioSpec want, have;
        SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
        SDL_memset(&have, 0, sizeof(have)); /* or SDL_zero(want) */

        video->FillAudioSpec(want);

        // Get fallback format for converter
        std::cout << "[Audio] Input AV format: " << video->GetAudioFormat() << std::endl;
        std::cout << "[Audio] Input channel layout: " << video->GetChannelsFormat() << std::endl;
        const auto FallbackFormat = P_AudioConverter::CompatibileFormat(video->GetAudioFormat());
        // const auto FallbackFormat = AUDIO_S32;
        std::cout << "[Audio] Fallback: " << FallbackFormat << std::endl;
        if(want.format == 0){ // if -1 then raw data won't work with sdl audio
            std::cout << "[Audio] Need to use audio converter" << std::endl;
            isAudioConverterUsed = 1;
            want.format = FallbackFormat; 
            if(FallbackFormat == 0){
                std::cout << "[Audio][ERR] Couldn't find any compatible format to convert" << std::endl;
                return -20;
            }
            std::cout << "[Audio] Input format: " << av_get_sample_fmt_name(
                video->GetAudioFormat()
            ) << std::endl;
            std::cout << "[Audio] Output format(" << want.format << "): " 
                << av_get_sample_fmt_name(P_Translation::AudioSDLFormat(want.format)) << std::endl;
        }else{
            std::cout << "[Audio] Using raw data" << std::endl;
            std::cout << "[Audio] Input/output format(" << want.format << "): " << av_get_sample_fmt_name(
                P_Translation::AudioSDLFormat(want.format)
            ) << std::endl;
        }

        SDL_AudioDeviceID tmp;
        if ((tmp = SDL_OpenAudioDevice(AudioTarget, 0, &want, &have, 0)) == 0) {
            std::cout << "[Audio][ERR] Failed to open audio: " << SDL_GetError() << std::endl;
        } else {
            std::cout << "[Audio] Output opened: " << tmp << std::endl;
            AudioTargetID = tmp;

            if (have.format != want.format) {
                SDL_Log("We didn't get wanted audio format.");
            }
            AudioBuffer = (uint8_t*) malloc(AudioBufferSize);
            memset(AudioBuffer, 0, AudioBufferSize);

            std::cout << "[Audio] Freq: " << want.freq << "\t" << have.freq << std::endl;
            std::cout << "[Audio] Format: " 
                << av_get_sample_fmt_name(
                    P_Translation::AudioSDLFormat(want.format)
                ) << "\t" 
                << av_get_sample_fmt_name(
                    P_Translation::AudioSDLFormat(have.format)
                ) << std::endl;
            std::cout << "[Audio] Channels: " << (int) want.channels << "\t" << (int) have.channels << std::endl;
            std::cout << "[Audio] Samples: " << want.samples << "\t" << have.samples << std::endl;
            SDL_PauseAudioDevice(AudioTargetID, 0); /* start audio playing. */
            
            if(isAudioConverterUsed){

                const auto ConvTarget = P_Translation::AudioSDLFormat(have.format);
                if(ConvTarget == AV_SAMPLE_FMT_NONE){
                    std::cout << "Converter doesn't support given audio format" << std::endl
                        << "Wanted: " << want.format  << std::endl
                        << "\t Bitsize: " << SDL_AUDIO_BITSIZE(want.format) << std::endl
                        << "\t Float: " << (SDL_AUDIO_ISFLOAT(want.format)?"Yes":"No") << std::endl
                        << "Have: " << have.format << std::endl
                        << "\t Bitsize: " << SDL_AUDIO_BITSIZE(have.format) << std::endl
                        << "\t Float: " << (SDL_AUDIO_ISFLOAT(have.format)?"Yes":"No") << std::endl;
                    return -1;
                }

                AudioConverter.Init(
                    video->GetAudioFormat(), want.freq, video->GetChannelsFormat(),
                    ConvTarget, have.freq, video->GetChannelsFormat()
                );
            }
        }



        return 0;
    }

    int OpenVideo(){
        int status;
        const int width = video->GetWidth();
        const int height = video->GetHeight();

        SurfaceTargetFormat = P_Translation::PixAVFormat(video->GetPixelFormat());

        if(SurfaceTargetFormat == SDL_PIXELFORMAT_UNKNOWN){
            const AVPixelFormat FallbackFmt = P_ImageConverter::BestFallback(video->GetPixelFormat());

            std::cout << "[Open Vid] Can't translate SDL format: " << av_get_pix_fmt_name(video->GetPixelFormat()) << std::endl;
            std::cout << "[Open Vid] Fallback: " << av_get_pix_fmt_name(FallbackFmt) << std::endl;
            status = ImageConverter.Init(width, height, video->GetPixelFormat(),
                width, height, FallbackFmt);
            if(status != 0){
                std::cout << "[Open Vid][ERR] Couldn't initilize converter: " << status << std::endl; 
                return -2;
            }
            SurfaceTargetFormat = P_Translation::PixAVFormat(FallbackFmt);
            std::cout << "[Open Vid] TargetFormat: " << SDL_GetPixelFormatName(SurfaceTargetFormat) << std::endl; 
        }
        return 0;
    }

    int Open(){   
        int status;
        status = video->OpenFile();
        if(status != 0){
            std::cout << "[Open Vid] Couldn't open file" << std::endl; 
            return -1;
        }
        
        status = OpenVideo();
        if(status != 0){
            std::cout << "[Open Vid] Couldn't open video" << std::endl; 
            return -2;
        }

        status = OpenAudio();
        if(status != 0){
            std::cout << "[Open Vid] Couldn't open audio" << std::endl; 
            return -3;
        }

        return 0;
    }

    int CreateWinow(){
        int status;
        window = std::make_unique<P_Window>(video->GetWidth(), 
            video->GetHeight(), "Player", SurfaceTargetFormat);
        status = window->Create();
        if(status < 0){
            return -1;
        }
        return 0;
    }

    int HandleAudio(P_Frame &frame){
        int status;
        const int size = video->GetAudioBufferSize(frame.nb_samples());
        // std::cout << "[MAIN] Samples Amount: " << frame.nb_samples() << std::endl;
        // std::cout << "[MAIN] Buffer size: " << size << std::endl;

        if(size < 0){
            std::cout << "[MAIN] Couldn't calculate audio buffer size" << std::endl;
            return -5;
        }

        if(isAudioConverterUsed && frame.data()[0] != nullptr){           
            status = AudioConverter.Convert(
                (const uint8_t**) frame.data(), size, &AudioBuffer, AudioBufferSize
            );
            if(status < 0){
                std::cout << "[Audio] Couldn't convert audio" << std::endl;
                return -6;
            }

            status = SDL_QueueAudio(AudioTargetID, 
                AudioBuffer, status);
            if(status < 0){
                std::cout << "[Audio] Can't queue audio data:\n\t"
                    << SDL_GetError() << std::endl; 
            }
        }else{
            status = SDL_QueueAudio(AudioTargetID, 
                frame.data()[0], size);
            if(status != 0){
                std::cout << "[Audio] Can't queue audio data:\n\t"
                    << SDL_GetError() << std::endl; ;
            }
        }
        return 0;
    }

    int HandleVideo(P_Frame &frame, int& foundVideoFrame){
        int status;
        ImageConverter.Convert(frame);

        void *data = nullptr;
        int pitch = 0;
        status = window->Lock(&data, &pitch);
        if(status < 0){
            std::cout << "[MAIN] Couldn't lock windows texture" << std::endl;
            return -2;
        }

        if(data != nullptr && pitch != 0){
            ImageConverter.CopyTo(data, pitch, video->GetHeight(), SurfaceTargetFormat);
        }else{
            std::cout << "[MAIN] Couldn't copy frame to texture" << std::endl;
            return -3;
        }

        status = window->UnlockAndRender();
        if(status < 0){
            std::cout << "[MAIN] Couldn't unlock and render frame" << std::endl;
            return -4;
        }
        foundVideoFrame=1;
        return 0;
    }

    int HandleFrame(){
        int status;
        AVMediaType type;
        int got_frame;
        
        for(int foundVideoFrame=0;foundVideoFrame==0;){
            P_Frame frame;
            status = video->DecodeFrame(type, got_frame, frame);
            if(status != 0){
                std::cout << "Can't decode frame" << std::endl;
                return -1;
            }

            if(got_frame == 1 && type == AVMEDIA_TYPE_VIDEO){
                status = HandleVideo(frame, foundVideoFrame);
                if(status < 0){
                    return status;
                }
            }else if(got_frame == 1 && type == AVMEDIA_TYPE_AUDIO){
                status = HandleAudio(frame);
                if(status < 0){
                    return status;
                }
            }
        }

        return 0;
    }

    int HandleEvents(const int timeout){    
        int status;
        static SDL_Event ev;

        uint32_t processingStart = SDL_GetTicks();
        uint32_t processingLast = processingStart;

        #define processingTime (processingLast-processingStart)
        while(processingTime < timeout){
            status = SDL_WaitEventTimeout(NULL, timeout-processingTime);
            if(status == 1){
                while(SDL_PollEvent(&ev)){
                    switch (ev.type)
                    {
                    case SDL_QUIT:
                        return 1;
                        break;
                    
                    default:
                        break;
                    }
                }
            }else{
                break;
            }
            processingLast = SDL_GetTicks();
        }

        return 0;
    }
};

typedef const uint32_t Time;

int main(int argc, char* argv[]){
    int status;

    if(argc != 2){
        std::cout << "Usage: " << argv[0] << " <path to video>" << std::endl; 
        return -15;
    }

    std::cout << "Starting player" << std::endl;
    status = SDL_Init(
        SDL_INIT_VIDEO | SDL_INIT_EVENTS
        | SDL_INIT_TIMER | SDL_INIT_AUDIO);
    if(status != 0){
        std::cout << "[MAIN][ERR] Can't initilize SDL2 " << std::endl;
        return -255;
    }

    std::string VideoPath(argv[1]);
//    std::string VideoPath("P:\\Arrow Season 8\\Arrow.S08E01.Starling.City.1080p.10bit.AMZN.WEB-DL.AAC5.1.HEVC-Vyndros.mkv");

    if(std::filesystem::exists(VideoPath)){
        std::cout << "[Input] Input file exists" << std::endl;
    }else{
        std::cout << "[Input] Input file doesn't exists" << std::endl;
        return -1;
    }

    MainCTX CTX(VideoPath, "NCX");

    status = CTX.Open();
    if(status < 0){
        std::cout << "[MainCTX] Error while opening video file" << std::endl;
        return -1;
    }
    status = CTX.CreateWinow();
    if(status < 0){
        std::cout << "[MainCTX] Error while creating window" << std::endl;
        return -2;
    }

    const int loopTime = 41; // Loop time in miliseconds

    const int forcetime = 30*60;
    // const int forcetime = 20;
    for(int i=0;i<forcetime;i++){
        // std::cout << "Frame: " << i << std::endl;
        Time T_FrameBegin = SDL_GetTicks();
        status = CTX.HandleFrame();
        if(status < 0){
            return -3;
        }
        Time T_VideoHandled = SDL_GetTicks();

        auto restTime = loopTime-(T_VideoHandled - T_FrameBegin);
        if(restTime > loopTime){
            std::cout << "[MAIN] Loop time error: " << i << std::endl;
            restTime = loopTime;
        }
        status = CTX.HandleEvents(restTime);
        Time T_FrameEnd = SDL_GetTicks();

        // std::cout << "[FULL] (" << T_FrameEnd - T_FrameBegin
        //  << ") [Video] (" << T_VideoHandled - T_FrameBegin << ")"
        //   <<  std::endl;
        if(status == 1){
            break;
        }
        if(status < 0){
            return -4;
        }
    }

    SDL_Quit();
    return 0;
}