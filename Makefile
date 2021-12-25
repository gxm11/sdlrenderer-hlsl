cc = g++
target = main.exe 
source = sdlrenderer-hlsl.cpp
msys64 = D:/Software/msys64
include = -I.
# include += -I./src/render

libs = -L${msys64}/mingw64/lib
libs += -lstdc++
libs += -lmingw32 -lSDL2main -lSDL2
libs += -ldinput8 -lshell32 -lsetupapi -ladvapi32 -luuid -lversion -loleaut32 -lole32 -limm32 -lwinmm -lgdi32 -luser32 -lm
cflags = -I${msys64}/mingw64/include/SDL2 -Dmain=SDL_main
cflags += -std=c++20 -O3 -m64 -mwindows


all: $(target)

$(target) : $(source)
	$(cc) $(include) $(source) -o $(target) $(cflags) -static $(libs)
	# upx $(target)