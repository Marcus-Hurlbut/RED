// Marcus Hurlbut - Vulkan Renderer

#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

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
#include <fstream>



#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define CLAMP(x, lo, hi)    ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))

#define SHADER_VERT_FILE_DIR "/src/shaders/vert.spv"
#define SHADER_FRAG_FILE_DIR "/src/shaders/frag.spv"


class Renderer
{
public:
	Renderer();
	~Renderer();

private:
	// Application Deubuger Mode
	bool debug_mode = true;

	// SDL Window
	GLFWwindow* window;
	//SDL_WindowFlags window_flags;
	//SDL_Event event;

	// Vulkan Setup Components 
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
	VkSwapchainKHR swap_chain = VK_NULL_HANDLE;					// Swap Chain
	VkFormat swap_chain_image_format;							// Format of swapchain
	VkExtent2D swap_chain_extent;								// Extent / resolution
	VkPipelineLayout pipelineLayout;							// Pipeline for rendering
	VkPipeline graphicsPipeline;
	VkRenderPass render_pass;									// Renderer Pass
	VkCommandPool commandPool;									// Command pool
	VkCommandBuffer commandBuffer;								// Command Buffer

	// Vulkan Buffers
	std::vector <VkImage> swapChainImages;						// Images in swap chain
	std::vector <VkImageView> swapChainImageViews;				// Image views
	std::vector <VkFramebuffer> swapChainFrameBuffers;			// Frame Buffesr

	// Synchronization objects
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;


	// Validation Layers for Vulkan Elementsdf
	const bool enableValidationLayers = true;
	const std::vector <const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};		// Validation layers for instance & device
	//std::vector <const char*> SDL_extensions{};													// SDL extensions
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
	void eventHandler();					// Main loop for application

	VkResult errorHandler(VkResult error);	// Error Handling for Vulkan results
	void createWindow();					// Initialize and setup SDL window
	void createInstance();					// Initialize Vulkan Application instance
	std::vector<const char*> checkSDLExtensions();				// Connect SDL Extensions to Vulkan Application
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
	void setSwapChainProp(SwapChainProperties& swapChainProperties);					// Fill SwapChain Properties
	void createImageViews();


	std::vector<char> readFile(const std::string &fileName);						// Reads in Files
	VkShaderModule createShaderModule(std::vector<char> &buffer);						// Create Module from Shader Files
	void createGraphicsPipeline();														// Graphics Pipeline for Rendering
	void createRenderPass();															// Create the Renderpass for Frame bufers
	void createFrameBuffers();															// Create Frame Buffers for Rendering
	void createCommandPool();
	void createCommandBuffer();															// Create Command Buffer
	void writeCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index);		// Writes to Command buffers

	void createSyncObjects();
	void drawFrame();																	// Draws each Frame
};