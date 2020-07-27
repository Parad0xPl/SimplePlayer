#include "P_VideoFile.hpp"

#include <iostream>
#include <cstdlib>

#include "P_Frame.hpp"
#include "P_AudioConverter.hpp"
#include "P_Translation.hpp"

const int SDL_AUDIO_BUFFER_SIZE = (1 << (10+4));

P_VideoFile::P_VideoFile(std::string &filepath)
    :path(filepath),
    CTX(nullptr),
    VideoCodec(nullptr),
    VideoCodecCTX(nullptr),
    AudioCodec(nullptr),
    AudioCodecCTX(nullptr){
}

P_VideoFile::~P_VideoFile(){
    std::cout << "[PV] Destructing" << std::endl;
    if(CTX != nullptr){
        avformat_close_input(&CTX);
        // avformat_free_context(CTX);
        CTX = nullptr;
    }

    if(AudioCodecCTX != nullptr){
        avcodec_free_context(&AudioCodecCTX);
    }
    if(VideoCodecCTX != nullptr){
        avcodec_free_context(&VideoCodecCTX);
    }
}

int P_VideoFile::OpenFile(){
    int status;

    std::cout << "[PV] Opening: " << path << std::endl;
    status = avformat_open_input(&CTX, path.c_str(), nullptr, nullptr);
    if(status != 0){
        std::cout << "[PV][ERR] Couldn't open file"  << std::endl;
        return -1;
    }

    // Find stream audio if needed
    status = avformat_find_stream_info(CTX, nullptr);
    if(status < 0){
        std::cout << "[PV][ERR] Couldn't read stream info"  << std::endl;
        // return -2;
    }

    // Find best audio stream
    status = av_find_best_stream(CTX, AVMEDIA_TYPE_AUDIO, -1, -1, &AudioCodec, 0);
    if(status < 0){
        std::cout << "[PV][ERR] Couldn't find best audio stream"  << std::endl;
        // return -3;
    }
    AudioStreamID = status;
    
    // Find best video stream
    status = av_find_best_stream(CTX, AVMEDIA_TYPE_VIDEO, -1, -1, &VideoCodec, 0);
    if(status < 0){
        std::cout << "[PV][ERR] Couldn't find best video stream"  << std::endl;
        // return -4;
    }
    VideoStreamID = status;

    if(AudioStreamID < 0 && VideoStreamID < 0){
        std::cout << "[PV][ERR] Didn't find any audio and video stream"  
            << std::endl;
        return -5;
    }

    status = AllocCodecCTX();
    if(status < 0){
        std::cout << "[PV][ERR] Couldn't allocate CodecCTX" << std::endl;
        return -6;
    }

    std::cout << "[PV] Image Format: " << av_get_pix_fmt_name(VideoCodecCTX->pix_fmt) << std::endl;
    std::cout << "[PV] Audio Format: " << av_get_sample_fmt_name(AudioCodecCTX->sample_fmt) << std::endl;

    return 0;
}

int P_VideoFile::AllocCodecCTX(){
    if(AudioStreamID >= 0){
        AudioCodecCTX = avcodec_alloc_context3(AudioCodec);
        if(AudioCodecCTX == nullptr){
            std::cout << "Couldn't alloc AudioCodec context" << std::endl;
            return -1;
        }

        AVDictionary *audioOpts = nullptr;
        if(avcodec_parameters_to_context(AudioCodecCTX, CTX->streams[AudioStreamID]->codecpar) < 0){
            std::cout << "Couldn't copy audio codec parameters" << std::endl;
            return -2;
        }
        if(avcodec_open2(AudioCodecCTX, AudioCodec, &audioOpts) < 0){
            std::cout << "Couldn't open AudioCodec context" << std::endl;
            return -3;
        }
    }
    if(VideoStreamID >= 0){
        VideoCodecCTX = avcodec_alloc_context3(VideoCodec);
        if(VideoCodecCTX == nullptr){
            std::cout << "Couldn't alloc VideoCodec context" << std::endl;
            return -1;
        }
        
        AVDictionary *videoOpts= nullptr;
        if(avcodec_parameters_to_context(VideoCodecCTX, CTX->streams[VideoStreamID]->codecpar) < 0){
            std::cout << "Couldn't copy video codec parameters" << std::endl;
            return -2;
        }
        if(avcodec_open2(VideoCodecCTX, VideoCodec, &videoOpts) < 0){
            std::cout << "Couldn't open VideoCodec context" << std::endl;
            return -3;
        }
    }

    return 0;
}

int P_VideoFile::DecodeFrame(AVMediaType &type, int& got_frame, P_Frame &frame){
    got_frame = 0;
    type = AVMEDIA_TYPE_UNKNOWN;

    AVPacket *packet = av_packet_alloc();
    if(packet == nullptr){
        std::cout << "[PV][ERR] Couldn't alloc packet" << std::endl;
        return -255;
    }
    // P_Frame frame;
    int status;

    av_init_packet(packet);
    packet->data = NULL;
    packet->size = 0;

    status = av_read_frame(CTX, packet);
    if(status != 0){
        std::cout << "[PV][ERR] Couldn't decode frame"  << std::endl;
        av_packet_free(&packet);
        return -1;
    }

    int retcode = 0;

    if(packet->stream_index == AudioStreamID){
        type = AudioCodec->type;
        status = avcodec_send_packet(AudioCodecCTX, packet);
        if(status < 0){
            std::cout << "[PV][ERR] Couldn't send packet to audio decoder: " 
                << status << std::endl;
            retcode = -2;
            goto end;
        }
        status = avcodec_receive_frame(AudioCodecCTX, frame.ptr());
        if(status == AVERROR(EAGAIN) || status == AVERROR_EOF){
            std::cout << "[PV] No audio frame to receive" << std::endl;
        }else if(status < 0){
            std::cout << "[PV][ERR] Couldn't receive frame to audio decoder: " 
                << status << std::endl;
            retcode = -3;
            goto end;
        }else if (status == 0){
            got_frame = 1;
            ProcessAudioFrame(frame);
        }
    }else if(packet->stream_index == VideoStreamID){
        type = VideoCodec->type;
        status = avcodec_send_packet(VideoCodecCTX, packet);
        if(status < 0){
            std::cout << "[PV][ERR] Couldn't send packet to video decoder: " 
                << status << std::endl;
            retcode = -4;
            goto end;
        }
        status = avcodec_receive_frame(VideoCodecCTX, frame.ptr());
        if(status == AVERROR(EAGAIN) || status == AVERROR_EOF){
            std::cout << "[PV] No video frame to receive" << std::endl;
        }else if(status < 0){
            std::cout << "[PV][ERR] Couldn't receive frame to video decoder: "
                << status << std::endl;
            retcode = -5;
            goto end;
        }else if (status == 0){
            got_frame = 1;
            ProcessVideoFrame(frame);
        }
    }
    
    end:
    av_packet_free(&packet);
    return retcode;
}

int P_VideoFile::FillAudioSpec(SDL_AudioSpec &spec){
    // std::cout << "Format: " << av_get_sample_fmt_name(AudioCodecCTX->sample_fmt) 
    //     << "(" << AudioCodecCTX->sample_fmt << ")" << std::endl;

    spec.freq = AudioCodecCTX->sample_rate;
    spec.format = P_Translation::AudioAVFormat(AudioCodecCTX->sample_fmt);
    spec.channels = AudioCodecCTX->channels;
    // spec.samples = 8192;
    spec.samples = 4096;
    spec.callback = nullptr;
    return 0;
}

int P_VideoFile::GetAudioBufferSize(int nb_samples){
    return av_samples_get_buffer_size(
        nullptr, AudioCodecCTX->channels, nb_samples, 
        AudioCodecCTX->sample_fmt, 1
    );
}

void P_VideoFile::ProcessAudioFrame(P_Frame &frame){
    // std::cout << "[A] Sample Rate: " << frame.ptr()->sample_rate << std::endl;
    // std::cout << "[A] Channels: " << frame.ptr()->channels << std::endl;
}

void P_VideoFile::ProcessVideoFrame(P_Frame &frame){
    int status;
    // std::cout << "[V] Video Frame: " << VideoCodecCTX->frame_number << std::endl;
    // std::cout << "[V] Width: " << frame.ptr()->width << std::endl;
    // std::cout << "[V] Height: " << frame.ptr()->height << std::endl;
    // std::cout << "[V] PixelFormat: "
        //  << av_get_pix_fmt_name(static_cast<enum AVPixelFormat>(
        //     frame.ptr()->format)) << std::endl;

    // status = ImageConverter.Convert(frame);
    if(status < 0){
        return;
    }
}

int P_VideoFile::GetWidth(){
    if(VideoCodecCTX){
        return VideoCodecCTX->width;
    }
    std::cout << "Didn't return width" << std::endl;
    return 0;
}

int P_VideoFile::GetHeight(){
    if(VideoCodecCTX){
        return VideoCodecCTX->height;
    }
    std::cout << "Didn't return height" << std::endl;
    return 0;
    
}

AVPixelFormat P_VideoFile::GetPixelFormat(){
    if(VideoCodecCTX != nullptr){
        return VideoCodecCTX->pix_fmt;
    }
    return AV_PIX_FMT_NONE;
}

AVSampleFormat P_VideoFile::GetAudioFormat(){
    if(AudioCodecCTX != nullptr){
        return AudioCodecCTX->sample_fmt;
    }
    return AV_SAMPLE_FMT_NONE;    
}

uint64_t P_VideoFile::GetChannelsFormat(){
    if(AudioCodecCTX != nullptr){
        return AudioCodecCTX->channel_layout;
    }
    return AV_SAMPLE_FMT_NONE;
}