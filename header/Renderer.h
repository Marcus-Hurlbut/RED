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
	VkDebugUtilsMessengerEXT debugMessenger;					// Vulkan Debugger
	VkPhysicalDevice gpu = VK_NULL_HANDLE;						// Physical representation of GPU
	VkDevice device = VK_NULL_HANDLE;							// Logical Device connected to GPU
	uint32_t graphics_family_index = 0;							// Graphics Family
	VkResult result;

	std::vector <const char*> layers{};							// Instance layer string enums
	std::vector <const char*> SDL_extensions{};			// Instance extension enums
	std::vector <const char*> device_extensions{};				// Device extension enums

	VkDebugReportCallbackEXT debug_report = VK_NULL_HANDLE;		// Debugger callback report


public: // Delete 'public' later *
	void initVulkan();
	void deInitVulkan();
	
	void createInstance();		// Initialize Vulkan Application instance
	void createDebug();			// Initialize Debugger	
	void createSDLWindow();		// Initialize and setup SDL window
	void setupDevice();			// Initialize & Create physical device & queue families for Setup
	void createDevice();		// Setup Vulkan GPU Device with 

	VkResult errorHandler(VkResult error);	// Error Handling for Vulkan results

};