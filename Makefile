cc = g++
target = main.exe 
source = sdlrenderer-hlsl.cpp

include = -I.
include += -I./src/render
# include += -ID:/Software/msys64/mingw64/include
# include += -ID:/Software/msys64/mingw64/x86_64-w64-mingw32/include

libs = -LD:/Software/msys64/mingw64/lib
libs += -lstdc++
libs += -lmingw32 -lSDL2main -lSDL2
libs += -ldinput8 -lshell32 -lsetupapi -ladvapi32 -luuid -lversion -loleaut32 -lole32 -limm32 -lwinmm -lgdi32 -luser32 -lm
cflags = -ID:/Software/msys64/mingw64/include/SDL2 -Dmain=SDL_main
cflags += -std=c++20 -O3


all: $(target)

$(target) : $(source)
	$(cc) $(include) $(source) -o $(target) $(cflags) -static $(libs)
