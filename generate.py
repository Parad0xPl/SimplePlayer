#!/bin/env python3

# This script generate translation functions for SDL and FFMPEG types

class PictFormat:
    def __init__(self, _AV, _SDL):
        self.AV = _AV
        self.SDL = _SDL

    def AVtoSDL(self):
        return "  case AV_PIX_FMT_%s: return SDL_PIXELFORMAT_%s;\n" % (self.AV, self.SDL)
    def SDLtoAV(self):
        return "  case SDL_PIXELFORMAT_%s: return AV_PIX_FMT_%s;\n" % (self.SDL, self.AV)

class AudioFormat:
    def __init__(self, _AV, _SDL):
        self.AV = _AV
        self.SDL = _SDL

    def AVtoSDL(self):
        return "  case AV_SAMPLE_FMT_%s: return AUDIO_%s;\n" % (self.AV, self.SDL)
    def SDLtoAV(self):
        return "  case AUDIO_%s: return AV_SAMPLE_FMT_%s;\n" % (self.SDL, self.AV)

# Order: AV, SDL
PictFmts = [
    PictFormat("RGBA", "RGBA32"),
    PictFormat("ARGB", "ARGB32"),
    PictFormat("BGRA", "BGRA32"),
    PictFormat("ABGR", "ABGR32"),

    PictFormat("NV12", "NV12"),
    PictFormat("NV21", "NV21"),
]

# Order: AV, SDL
AudioFmts = [
    AudioFormat("U8", "U8"),
    AudioFormat("S16", "S16SYS"),
    AudioFormat("S32", "S32SYS"),

    AudioFormat("FLT", "F32SYS"),
]

def GeneratePicutreFormatAVtoSDL():
    with open("src/generated/PF_AVtoSDL.gensw", "w+") as f:
        f.write("switch(fmt){\n")
        for fmt in PictFmts:
            f.write(fmt.AVtoSDL())
        f.write("}\n")
    return

def GeneratePicutreFormatSDLtoAV():
    with open("src/generated/PF_SDLtoAV.gensw", "w+") as f:
        f.write("switch(fmt){\n")
        for fmt in PictFmts:
            f.write(fmt.SDLtoAV())
        f.write("}\n")
    return

def GenerateSoundFormatAVtoSDL():
    with open("src/generated/SF_AVtoSDL.gensw", "w+") as f:
        f.write("switch(fmt){\n")
        for fmt in AudioFmts:
            f.write(fmt.AVtoSDL())
        f.write("}\n")
    return

def GenerateSoundFormatSDLtoAV():
    with open("src/generated/SF_SDLtoAV.gensw", "w+") as f:
        f.write("switch(fmt){\n")
        for fmt in AudioFmts:
            f.write(fmt.SDLtoAV())
        f.write("}\n")
    return


GeneratePicutreFormatSDLtoAV()
GeneratePicutreFormatAVtoSDL()
GenerateSoundFormatAVtoSDL()
GenerateSoundFormatSDLtoAV()