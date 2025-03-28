@echo off
setlocal

@echo off

if not defined DevEnvDir (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" amd64
)
set SRC_DIR= src\\
set EXTERN= extern\\
set OPENAL_DIR= extern\\openal-soft
set SDL2_DIR=extern\\SDL2

set COMPILER_FLAGS=/nologo /Zi /MD /utf-8
set LINKER_FLAGS= /subsystem:console

@set OUT_DIR= Debug
@set OUT_EXE= Marshmallow
@set INCLUDES=/I%SDL2_DIR%\\include /I%EXTERN% /I%OPENAL_DIR%\\include
@set SOURCES= %SRC_DIR%sdl_marshmallow.cpp

@set SDL_LIBS=    /LIBPATH:%SDL2_DIR%\lib\x64 SDL2.lib SDL2main.lib
@set OPENAL_LIBS= /LIBPATH:extern\\openal-soft\\build\\Release OpenAL32.lib
@set LIBS=        %SDL_LIBS% %OPENAL_LIBS% kernel32.lib user32.lib shell32.lib

if not exist %OUT_DIR% mkdir %OUT_DIR%

cl %COMPILER_FLAGS% %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS% %LINKER_FLAGS%
