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
#include <set>
#include <algorithm>
#include <cstdint>
#include <iomanip>



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
	VkQueue present_queue;										// Queue for Presenting
	uint32_t queue_family_index = 0;							// Graphics Family indice
	uint32_t present_family_index = 0;
	VkDebugReportCallbackEXT debug_report = VK_NULL_HANDLE;		// Debugger callback report

	// Vulkan Presentation Components
	VkSurfaceKHR surface;										// Window Surface


	// Validation Layers for Vulkan Elements
	const bool enableValidationLayers = true;
	const std::vector <const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};		// Validation layers for instance & device
	std::vector <const char*> SDL_extensions{};													// SDL extensions
	std::vector <const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};			// Device extensions

	// Queue Family Indices for Physical Device
	struct QueueFamilyIndices 
	{
		uint32_t graphicsFamily = -1;
		uint32_t presentFamily = -1;

		bool hasEntry() { return (graphicsFamily != -1 && presentFamily != -1); }
	};

	// Swap Chain Properties for Surface
	struct SwapChainProperties
	{
		VkSurfaceCapabilitiesKHR extentCapabilities;
		std::vector <VkSurfaceFormatKHR> surfaceFormats;
		std::vector <VkPresentModeKHR> presentModes;
		VkExtent2D extent;
		VkSurfaceFormatKHR format;
		VkPresentModeKHR mode;
	};


public: // Delete 'public' later *
	void initVulkan();						// Initialize Vulkan App
	void deInitVulkan();					// DeInitialize Vulkan App
	VkResult errorHandler(VkResult error);												// Error Handling for Vulkan results
	void createWindow();					// Initialize and setup SDL window
	void createInstance();					// Initialize Vulkan Application instance
	void checkSDLExtensions();			// Connect SDL Extensions to Vulkan Application
	bool checkValidationLayers();			// Check for all Validation Layers

	VkResult createDebugMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,	// Instantiate Debugger Messenger Extension
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

	void destroyDebugMessengerEXT(VkInstance instance,																// Deallocate Debugger Messenger Extension
		VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	void createDebugMessenger();														// Initialize Debugger Messenger	 
	void insertDebugInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);				// Populate Debugger Messenger Content
	void createPhysicalDevice();														// Initialize & Create physical device
	void validatePhysicalDevice(bool &suitable, VkPhysicalDevice device);				// Validates Physical Device Suitability
	QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device);						// Find Queue Families for instanced device
	bool checkDeviceExtensions(VkPhysicalDevice device);								// Check for needed Device Extensions
	void createLogicalDevice();															// Create Logical Device from Physical GPU 
	void createSurface();																// Create Surface for graphics
	void createSwapChain();																// Create Swap Chain for
	SwapChainProperties querySwapChainProp(VkPhysicalDevice device);					// Query the Properties in Swap Chain
	void setSwapChainProp(SwapChainProperties& swapChainProperties);

};