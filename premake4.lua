#!lua

-- A solution contains projects, and defines the available configurations
solution "Player"
   configurations { "Debug", "Release" }

   -- A project defines one build target
   project "Player"
      kind "ConsoleApp"
      language "C++"
      files { "src/**.cpp" }
      buildoptions { 
        "-std=c++20"
      }
      linkoptions { 
        "`sdl2-config --cflags --libs`",
        "-std=c++20"
      }
      links { "avcodec", "avformat", "avutil"}
      links { "swscale", "swresample"}

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }