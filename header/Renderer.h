#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>
#include <optional>



#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 400


class Renderer
{
public:
	Renderer();
	~Renderer();

private:
	// Application Deubuger Mode
	bool debug_mode = true;

	// SDL Window
	SDL_Window* window;
	SDL_WindowFlags window_flags;

	// Vulkan Application Setup components 
	VkInstance instance;										// Vulkan Instance
	VkDebugUtilsMessengerEXT debug_messenger;					// Vulkan Debugger
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;			// Physical representation of GPU
	VkDevice device = VK_NULL_HANDLE;							// Logical Device connected to GPU
	VkQueue graphics_queue;										// Queue for Device (GPU)
	uint32_t queue_family_index = 0;							// Graphics Family indice
	VkDebugReportCallbackEXT debug_report = VK_NULL_HANDLE;		// Debugger callback report

	// Vulkan Presentation Components
	VkSurfaceKHR surface;										// Window Surface


	// Validation Layers for Vulkan Elements
	const bool enableValidationLayers = true;
	const std::vector <const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};		// Validation layers for instance & device
	std::vector <const char*> SDL_extensions{};													// SDL extensions
	std::vector <const char*> device_extensions{};												// Device extensions

	// Queue Family Index Search Structure
	struct QueueFamilyIndices 
	{
		uint32_t graphicsFamily = -1;
		bool hasEntry() { return graphicsFamily != -1; }
	};


public: // Delete 'public' later *
	void initVulkan();						// Initialize Vulkan App
	void deInitVulkan();					// DeInitialize Vulkan App
	void createWindow();					// Initialize and setup SDL window
	void createInstance();					// Initialize Vulkan Application instance
	void connectSDLExtensions();			// Connect SDL Extensions to Vulkan Application
	bool checkValidationLayers();			// Check for Validation Layers

	VkResult createDebugMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void destroyDebugMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	void createDebugMessenger();														// Initialize Debugger Messenger	 
	void insertDebugInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);				// Populate Debugger Messenger Content
	void createPhysicalDevice();														// Initialize & Create physical device
	void createLogicalDevice();															// Create Logical Device from Physical GPU
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);								// Find Queue Families for instanced device

	VkResult errorHandler(VkResult error);												// Error Handling for Vulkan results

	void createSurface();																// Create Surface for graphics

};