@echo off
setlocal

if not defined DevEnvDir (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" amd64
)
set SRC_DIR=src
set EXTERN=extern
set OPENAL_DIR=extern\\openal-soft
set SDL2_DIR=extern\\SDL2
set VULKAN_DIR=C:\VulkanSDK\1.4.309.0
set OPENGL_DIR=extern\\

set COMPILER_FLAGS=/nologo /Zi /MD /utf-8 /EHsc
set LINKER_FLAGS= /subsystem:console

set OUT_DIR=Debug
set OUT_EXE=Marshmallow
set INCLUDES=/I%SDL2_DIR%\\include /I%EXTERN% /I%OPENAL_DIR%\\include /I%VULKAN_DIR%\\include
set SOURCES=%SRC_DIR%\\sdl_marshmallow.cpp %EXTERN%\\glad.c

set SDL_LIBS=    /LIBPATH:%SDL2_DIR%\lib\x64 SDL2.lib SDL2main.lib
set OPENAL_LIBS= /LIBPATH:extern\\openal-soft\\build\\Release OpenAL32.lib
set VULKAN_LIBS= /LIBPATH:%VULKAN_DIR%\lib vulkan-1.lib
set LIBS=        %SDL_LIBS% %OPENAL_LIBS% %VULKAN_LIBS% kernel32.lib user32.lib shell32.lib opengl32.lib

if not exist %OUT_DIR% mkdir %OUT_DIR%

cl %COMPILER_FLAGS% %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS% %LINKER_FLAGS%
