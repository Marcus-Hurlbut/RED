#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>


#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 400


class Renderer
{
public:
	Renderer();
	~Renderer();

private:
	// SDL
	SDL_Window* window;
	SDL_WindowFlags window_flags;

	// Vulkan 
	VkInstance instance = VK_NULL_HANDLE;						// Vulkan Instance
	VkDevice device = VK_NULL_HANDLE;							// Logical Device connected to GPU
	VkPhysicalDevice gpu = VK_NULL_HANDLE;						// Physical representation of GPU
	uint32_t graphics_family_index = 0;							// Graphics Family

	std::vector <const char*> layers{};						// Instance layer string enums
	std::vector <const char*> instance_extensions{};			// Instance extension enums
	std::vector <const char*> device_extensions{};				// Device extension enums

	VkDebugReportCallbackEXT debug_report = VK_NULL_HANDLE;		// Debugger callback report


public: // Delete 'public' later *
	void errorHandler(VkResult error);	// Error Handling for Vulkan results

	void initWindow();					// Initialize SDL window
	void initInstance();				// Initialize Vulkan Application instance
	void setupDevice();					// Setup physical device & queue families for initialization
	void initDevice();					// Initialize Vulkan GPU Device
	void initDebug();					// Initialize Debugger
	void initSurface();

	void deleteWindow();				// Deinitialize Window
	void deleteInstance();				// Deinitialize Instance
	void deleteDebug();					// Deinitalize Debugger
	void deleteDevice();				// Deinitialize Device			

};