// Marcus Hurlbut - Vulkan Renderer

#include "Renderer.h"


// Constructor & Deconstructors
Renderer::Renderer()
{
	initVulkan();
}

Renderer::~Renderer()
{
	deInitVulkan();
}


// Initializers & Deinitializers
void Renderer::initVulkan()
{
	createWindow();
	createInstance();
	createDebugMessenger();
	createSurface();
	createPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createGraphicsPipeline();
}


void Renderer::deInitVulkan()
{
	// Destroy Image Views
	for (auto imageView : swapChainImageViews) 
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

	// Destroy Swap Chain
	vkDestroySwapchainKHR(device, swap_chain, nullptr);

	// Destroy device
	vkDestroyDevice(device, nullptr);
	device = VK_NULL_HANDLE;

	// Destroy Debugger Report
	if (enableValidationLayers)
	{
		destroyDebugMessengerEXT(instance, debug_messenger, nullptr);
		debug_report = VK_NULL_HANDLE;
	}

	// Destroy Surface
	vkDestroySurfaceKHR(instance, surface, nullptr);

	// Destroy Instance
	vkDestroyInstance(instance, nullptr);
	instance = nullptr;

	// Destroy SDL Window and Quit SDL
	SDL_DestroyWindow(window);
	void SDL_Quit(void);
}


// SDL Window Initializaiton and Creation
void Renderer::createWindow()
{
	// Initialize SDL 
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Vulkan Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_VULKAN);

	if (window == nullptr)
	{
		std::cout << "\n[!] SDL Error: Unable to initilaize SDL window.\n";
		std::exit(-1);
	}
}


// Create a Vulkan Instance
void Renderer::createInstance()
{
	// Get all Instance Layers
	uint32_t layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties> layer_properties(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data());


	// Check to ensure Support for Validation Layers
	if (enableValidationLayers && !checkValidationLayers())
	{
		std::cout << "\n[!] Validation Error: layers Requested but NOT Found.";
		std::exit(-1);
	}

	// Set Application Info
	VkApplicationInfo application {};
	application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application.pApplicationName = "Vulkan Renderer Prototype";
	application.apiVersion = VK_API_VERSION_1_0;
	application.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	application.pEngineName = "No Engine";

	// Instantiate Vulkan from Application Info and SDL Extensions
	VkInstanceCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &application;
	checkSDLExtensions();
	create_info.enabledExtensionCount = static_cast<uint32_t>(SDL_extensions.size());
	create_info.ppEnabledExtensionNames = SDL_extensions.data();

	// Validate Instance Layers
	VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
	if (enableValidationLayers)
	{
		create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		create_info.ppEnabledLayerNames = validationLayers.data();

		insertDebugInfo(debug_create_info);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
	}
	else 
	{
		create_info.enabledLayerCount = 0;
		create_info.pNext = nullptr;
	}

	// Instance Error Handling
	if (errorHandler(vkCreateInstance(&create_info, nullptr, &instance)) != VK_SUCCESS)
	{
		std::cout << "[!] Failed to Create a Vulkan Instance.";
		std::exit(-1);
	}
}


// Check for Validation Layers Support in Instance
bool Renderer::checkValidationLayers()
{
	// Get Instance Layer Property Enums for iteration
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector <VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	bool layerFound = false;

	// Check Validation Layers
	for (const char* layerName : validationLayers)
	{
		layerFound = false;
		// Iterate and Check through Layer Layers
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
		{
			std::cout << "\n[!] " << layerName << " Validation Layer NOT Found\n";
			return false;
		}
	}
	return true;
}


// Connect SDL Extensions to Vulkan Application
void Renderer::checkSDLExtensions()
{
	// Get SDL Extensions required to make Surface in Vulkan
	uint32_t extension_count = 0;
	SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);
	SDL_extensions.resize(extension_count);
	SDL_Vulkan_GetInstanceExtensions(window, &extension_count, SDL_extensions.data());

	if (enableValidationLayers)
	{
		SDL_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
}


// Initialize Debug Messenger Extension
VkResult Renderer::createDebugMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	};
}


// Destroy Debug Messenger Extension
void Renderer::destroyDebugMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}


// Vulkan Debug Callback Function
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "\nDebug Callback - Validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}


// Initialize Debugger
void Renderer::createDebugMessenger()
{
	if (!enableValidationLayers) return;
	
	VkDebugUtilsMessengerCreateInfoEXT create_info{};
	insertDebugInfo(create_info);
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = debugCallback;  

	if (createDebugMessengerEXT(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS)
	{
		throw std::runtime_error("[!] Failed to setup Debug messenger.");
		std::exit(-1);
	}

}

// Create and Initialize Debug Messenger
void Renderer::insertDebugInfo(VkDebugUtilsMessengerCreateInfoEXT& info)
{
	info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	info.pfnUserCallback = debugCallback;
}


// Setup physical device & queue families for initialization
void Renderer::createPhysicalDevice()
{
	// Get Physical Device - GPU
	uint32_t gpu_count = 0;
	vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
	std::vector<VkPhysicalDevice> physical_devices(gpu_count);
	vkEnumeratePhysicalDevices(instance, &gpu_count, physical_devices.data());
	
	// Physical Device Not Found Error
	if (gpu_count == 0)
	{
		std::cout << "[!] Vulkan Error: Failed to find a supporting GPU.";
		std::exit(-1);
	};

	// Check for a Supporting Device
	for (const auto& device : physical_devices)
	{
		bool isSuitable = false;
		validatePhysicalDevice(isSuitable, device);
		if (isSuitable)
		{
			physical_device = device;
			break;
		}
	}

	// Device Validation Error
	if (physical_device == VK_NULL_HANDLE) 
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

}


Renderer::QueueFamilyIndices Renderer::queryQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	// Initialize Queue Family
	uint32_t family_count = 0;
	std::vector<VkQueueFamilyProperties> queue_families;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);
	queue_families.resize(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, queue_families.data());

	// Find a Supporting Queue Family
	bool found = false;
	for (uint32_t i = 0; i < family_count; i++)
	{
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queue_family_index = i;
			indices.graphicsFamily = i;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
			present_family_index = i;
		}

		if (indices.hasEntry()) 
		{
			found = true;
			break;
		}
	}

	// Error Handling for Queue Family
	if (!found)
	{
		std::cout << "[!] Vulkan Error: Queue family for supporting graphics not found.";
		std::exit(-1);
	}

	return indices;
}


// Check for all Device Extensions
bool Renderer::checkDeviceExtensions(VkPhysicalDevice dev)
{
	// Lookup the available device extensions
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(dev, nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> device_extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(dev, nullptr, &extension_count, device_extensions.data());

	// Check against required extensions
	std::set <std::string> required_extensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : device_extensions)
	{
		required_extensions.erase(extension.extensionName);
	}

	return required_extensions.empty();
}


// Validate Physical Device Properties - Queue families & Extensions
void Renderer::validatePhysicalDevice(bool& suitable, VkPhysicalDevice device)
{
	QueueFamilyIndices indices = queryQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensions(device);

	bool supported_swap_chain = false;
	if (extensionsSupported) {
		SwapChainProperties swapChainSupport = querySwapChainProp(device);
		supported_swap_chain = !swapChainSupport.surfaceFormats.empty() && !swapChainSupport.presentModes.empty();
	}

	suitable = (indices.hasEntry() && extensionsSupported && supported_swap_chain);
}

// Initialize Vulkan Device
void Renderer::createLogicalDevice()
{
	QueueFamilyIndices indices = queryQueueFamilies(physical_device);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };
	float queue_priority[]{ 1.0f };

	// Iterate through all Queue Families for GPU
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		create_info.queueFamilyIndex = queueFamily;
		create_info.queueCount = 1;
		create_info.pQueuePriorities = queue_priority;
		queueCreateInfos.push_back(create_info);
	}

	// Specify device's features used with physical device - [!] Fill feature support in later when renderer advances 
	VkPhysicalDeviceFeatures device_features{};


	// Create Device Info - Logical Device
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	device_create_info.pQueueCreateInfos = queueCreateInfos.data();
	device_create_info.pEnabledFeatures = &device_features;
	device_create_info.enabledExtensionCount = static_cast<uint32_t> (deviceExtensions.size());		// [!] Fill in logical extensions later
	device_create_info.ppEnabledExtensionNames = deviceExtensions.data();		// [!] Fill in logical extensions later
	
	// Validation Layer Support for Logical Device
	if (enableValidationLayers)
	{
		device_create_info.enabledLayerCount = static_cast<uint32_t> (validationLayers.size());
		device_create_info.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		device_create_info.enabledLayerCount = 0;
	}

	// Logical device error handling
	if (errorHandler(vkCreateDevice(physical_device, &device_create_info, nullptr, &device)) != VK_SUCCESS)
	{
		std::cout << ("\n[!] Failed to Create Vulkan Logical Device");
		std::exit(-1);
	}

	// Get Logical Device Queue Handles
	vkGetDeviceQueue(device, queue_family_index, 0, &graphics_queue);
	vkGetDeviceQueue(device, present_family_index, 0, &present_queue);
}


// Initialize Window Surface
void Renderer::createSurface()
{
	if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE)
	{
		std::cout << "[!] Error: Failed to create vulkan surface window.";
		std::exit(-1);
	}

}


// Retrieve Swap Chain Property Info
Renderer::SwapChainProperties Renderer::querySwapChainProp(VkPhysicalDevice device)
{
	SwapChainProperties properties;
	uint32_t format_count;
	uint32_t mode_count;

	// Get Surface Capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &properties.extentCapabilities);

	// Get Surface Formats
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
	properties.surfaceFormats.resize(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, properties.surfaceFormats.data());

	// Get Presentation Mode
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, nullptr);
	properties.presentModes.resize(mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, properties.presentModes.data());

	return properties;
}


void Renderer::setSwapChainProp(SwapChainProperties& availableProperties)
{
	// Set Surface Format
	for (const auto& availableFormat : availableProperties.surfaceFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			availableProperties.format = availableFormat;
			break;
		}
		else
		{
			availableProperties.format = availableProperties.surfaceFormats[0];
		}
	}

	// Set Presentation Mode
	for (const auto& availablePresentMode : availableProperties.presentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			availableProperties.mode = availablePresentMode;
			break;
		}
		else
		{
			availableProperties.mode = VK_PRESENT_MODE_FIFO_KHR;
		}
	}

	// Set Resolution Extent Capabilities
	if (availableProperties.extentCapabilities.currentExtent.width != UINT32_MAX) 
	{
		availableProperties.extent = availableProperties.extentCapabilities.currentExtent;
	}
	else 
	{
		int width, height;
		SDL_Vulkan_GetDrawableSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		availableProperties.extent = actualExtent;
		std::cout << "[!] Resolution Error: Swap Chain Extent is set past the maximum!";
	}

}


void Renderer::createSwapChain()
{
	// Query Physical Device's Swap Chain Properties
	SwapChainProperties swapChainProperties = querySwapChainProp(physical_device);

	// Set Swap Chain Properties with available criteria
	setSwapChainProp(swapChainProperties);

	uint32_t image_count = swapChainProperties.extentCapabilities.minImageCount + 1;

	if (swapChainProperties.extentCapabilities.maxImageCount > 0 && image_count > swapChainProperties.extentCapabilities.maxImageCount)
	{
		image_count = swapChainProperties.extentCapabilities.maxImageCount;
	}

	// Create Swap Chain Info
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = image_count;
	createInfo.imageFormat = swapChainProperties.format.format;
	createInfo.imageColorSpace = swapChainProperties.format.colorSpace;
	createInfo.imageExtent = swapChainProperties.extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Get Queue Family Index for Swap Chain
	QueueFamilyIndices indices = queryQueueFamilies(physical_device);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

	// Graphics and Present Family Error Checking
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;							// [!] Might need to change later
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr; 
	}

	// Set Swap Chain Property Info
	createInfo.preTransform = swapChainProperties.extentCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = swapChainProperties.mode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Swap Chain Creation Error Handling
	if (errorHandler(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swap_chain)) != VK_SUCCESS)
	{
		std::cout << "[!] Swap Chain Error - Failed to create Swap Chain.";
		std::exit(-1);
	}

	// Retrieve Swap Chain Images
	vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr);
	swapChainImages.resize(image_count);
	vkGetSwapchainImagesKHR(device, swap_chain, &image_count, swapChainImages.data());

	swap_chain_image_format = swapChainProperties.format.format;
	swap_chain_extent = swapChainProperties.extent;
}


void Renderer::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) 
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swap_chain_image_format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (errorHandler(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i])) != VK_SUCCESS) 
		{
			std::cout << "failed to create image views!";
			std::exit(-1);
		}
	}

}


VkShaderModule Renderer::createShaderModule(std::vector<char> &buffer)
{
	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = buffer.size();
	create_info.pCode = reinterpret_cast<const uint32_t*> (buffer.data());

	if (errorHandler(vkCreateShaderModule(device, &create_info, nullptr, &shaderModule)) != VK_SUCCESS)
	{
		std::cout << "[!] Shader Module Error - Unable to create Shader module.";
		std::exit(-1);
	}
	return shaderModule;
}


void Renderer::createGraphicsPipeline()
{
	// Get Shader Vertices & Fragment from Buffer
	std::vector<char> shaderVert;
	std::vector<char> shaderFrag;
	if (!readFile(SHADER_VERT_FILE_DIR, shaderVert) || !readFile(SHADER_FRAG_FILE_DIR, shaderFrag))
	{
		throw std::runtime_error("Failed to read file");
		std::exit(-1);
	}

	// Create Shader Module
	auto shaderVertModule = createShaderModule(shaderVert);
	auto shaderFragModule = createShaderModule(shaderFrag);

	// Create Shader Vertices Stage
	VkPipelineShaderStageCreateInfo vert_create_info {};
	vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_create_info.module = shaderVertModule;
	vert_create_info.pName = "main";

	// Create Shader Fragment Stage
	VkPipelineShaderStageCreateInfo frag_create_info{};
	frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	frag_create_info.module = shaderFragModule;
	frag_create_info.pName = "main";

	// Create Shader Stages
	VkPipelineShaderStageCreateInfo stages[] = { vert_create_info, frag_create_info };

	// Destroy Shader Module 
	vkDestroyShaderModule(device, shaderFragModule, nullptr);
	vkDestroyShaderModule(device, shaderVertModule, nullptr);

}


bool Renderer::readFile(std::string fileName, std::vector<char>& buffer)
{
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open()) 
	{
		std::cout << "[!] File Error - failed to open and read in file: " << fileName;
		return false;
	}

	size_t fileSize = (size_t)file.tellg();
	buffer.resize(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return true;
}

// Handle Vulkan Result Errors
VkResult Renderer::errorHandler(VkResult error)
{
	switch (error)
	{
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		std::cout << "\n[!] Error: VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
		break;

	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		std::cout << "\n[!] Error: VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
		break;

	case VK_ERROR_INITIALIZATION_FAILED:
		std::cout << "\n[!] Error: VK_ERROR_INITIALIZATION_FAILED" << std::endl;
		break;

	case VK_ERROR_DEVICE_LOST:
		std::cout << "\n[!] Error: VK_ERROR_DEVICE_LOST" << std::endl;
		break;

	case VK_ERROR_MEMORY_MAP_FAILED:
		std::cout << "\n[!] Error: VK_ERROR_MEMORY_MAP_FAILED" << std::endl;
		break;

	case VK_ERROR_LAYER_NOT_PRESENT:
		std::cout << "\n[!] Error: VK_ERROR_LAYER_NOT_PRESENT" << std::endl;
		break;

	case VK_ERROR_EXTENSION_NOT_PRESENT:
		std::cout << "\n[!] Error: VK_ERROR_EXTENSION_NOT_PRESENT" << std::endl;
		break;

	case VK_ERROR_FEATURE_NOT_PRESENT:
		std::cout << "\n[!] Error: VK_ERROR_FEATURE_NOT_PRESENT" << std::endl;
		break;

	case VK_ERROR_INCOMPATIBLE_DRIVER:
		std::cout << "\n[!] Error: VK_ERROR_INCOMPATIBLE_DRIVER" << std::endl;
		break;

	case VK_ERROR_TOO_MANY_OBJECTS:
		std::cout << "\n[!] Error: VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
		break;

	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		std::cout << "\n[!] Error: VK_ERROR_FORMAT_NOT_SUPPORTED" << std::endl;
		break;

	case VK_ERROR_FRAGMENTED_POOL:
		std::cout << "\n[!] Error: VK_ERROR_FRAGMENTED_POOL" << std::endl;
		break;

	case VK_ERROR_UNKNOWN:
		std::cout << "\n[!] Error: VK_ERROR_UNKNOWN" << std::endl;
		break;

	case VK_ERROR_OUT_OF_POOL_MEMORY:
		std::cout << "\n[!] Error: VK_ERROR_OUT_OF_POOL_MEMORY" << std::endl;			
		break;

	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		std::cout << "\n[!] Error: VK_ERROR_INVALID_EXTERNAL_HANDLE" << std::endl;		
		break;

	case VK_ERROR_FRAGMENTATION:
		std::cout << "\n[!] Error: VK_ERROR_FRAGMENTATION" << std::endl;					
		break;

	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
		std::cout << "\n[!] Error: VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS" << std::endl;	
		break;

	case VK_ERROR_SURFACE_LOST_KHR:
		std::cout << "\n[!] Error: VK_ERROR_SURFACE_LOST_KHR" << std::endl;					
		break;

	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		std::cout << "\n[!] Error: VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" << std::endl;			
		break;

	case VK_SUBOPTIMAL_KHR:
		std::cout << "\n[!] Error: VK_SUBOPTIMAL_KHR" << std::endl;							
		break;

	case VK_ERROR_OUT_OF_DATE_KHR:
		std::cout << "\n[!] Error: VK_ERROR_OUT_OF_DATE_KHR" << std::endl;					
		break;

	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		std::cout << "\n[!] Error: VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" << std::endl;			
		break;

	case VK_ERROR_VALIDATION_FAILED_EXT:
		std::cout << "\n[!] Error: VK_ERROR_VALIDATION_FAILED_EXT " << std::endl;			
		break;

	case VK_ERROR_INVALID_SHADER_NV:
		std::cout << "\n[!] Error: VK_ERROR_INVALID_SHADER_NV" << std::endl;				
		break;

	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		std::cout << "\n[!] Error: VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT" << std::endl;	
		break;

	case VK_ERROR_NOT_PERMITTED_EXT:
		std::cout << "\n[!] Error: VK_ERROR_NOT_PERMITTED_EXT" << std::endl;					
		break;

	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		std::cout << "\n[!] Error: VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" << std::endl;		
		break;

	case VK_THREAD_IDLE_KHR:
		std::cout << "\n[!] Error: VK_THREAD_IDLE_KHR" << std::endl;							
		break;

	case VK_THREAD_DONE_KHR:
		std::cout << "\n[!] Error: VK_THREAD_DONE_KHR" << std::endl;					
		break;

	case VK_OPERATION_DEFERRED_KHR:
		std::cout << "\n[!] Error: VK_OPERATION_DEFERRED_KHR" << std::endl;					
		break;

	case VK_OPERATION_NOT_DEFERRED_KHR:
		std::cout << "\n[!] Error: VK_OPERATION_NOT_DEFERRED_KHR" << std::endl;			
		break;

	case VK_PIPELINE_COMPILE_REQUIRED_EXT:
		std::cout << "\n[!] Error: VK_OPERATION_NOT_DEFERRED_KHR" << std::endl;				
		break;

	default:
		return VK_SUCCESS;
		break;
	};

	return error;
}
