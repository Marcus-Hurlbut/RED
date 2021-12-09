OUT = VulkanTest
CXX = g++
SOURCE = -IC:\SDL_32bit\i686-w64-mingw32\include\SDL2 -IC:\SDL_ttf\include\SDL2 -IH:\Source_Libraries\Vulkan\Include -LC:\SDL_32bit\i686-w64-mingw32\lib -LC:\SDL_ttf\lib -LH:\Source_Libraries\Vulkan\Lib32 -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lvulkan-1


OBJECTS = main.o Renderer.o

all: $(OUT)
$(OUT): $(OBJECTS)
	$(CXX) -o $@ $^ ${SOURCE}

$(OBJECTS): Renderer.h

clean:
	del -f *.o
