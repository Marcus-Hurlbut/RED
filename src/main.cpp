
#include <windows.h>
#include <sstream>
#include <iostream>
#include "Renderer.h"

#define WIDTH 400
#define HEIGHT 400


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    Renderer vulkan;
    vulkan.eventHandler();

    return 0;
}

// mingw32-make -f Makefile
//  /home/user/VulkanSDK/x.x.x.x/x86_64/bin/glslc shader.vert -o vert.spv
//  /home/user/VulkanSDK/x.x.x.x/x86_64/bin/glslc shader.frag -o frag.spv


