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
	setupDevice();
	createDevice();
}


void Renderer::deInitVulkan()
{
	// Destroy device
	vkDestroyDevice(device, nullptr);
	device = VK_NULL_HANDLE;

	// Destroy Debugger Report
	if (enableValidationLayers)
	{
		destroyDebugMessengerEXT(instance, debug_messenger, nullptr);
		debug_report = VK_NULL_HANDLE;
	}

	// Destroy Instance
	vkDestroyInstance(instance, nullptr);
	instance = nullptr;

	// Destroy SDL Window
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
	connectSDLExtensions();
	create_info.enabledExtensionCount = static_cast<uint32_t>(SDL_extensions.size());
	create_info.ppEnabledExtensionNames = SDL_extensions.data();

	// Validate Instance Layers
	VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
	if (enableValidationLayers)
	{
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();

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
	for (const char* layerName : validation_layers)
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
void Renderer::connectSDLExtensions()
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
void Renderer::setupDevice()
{
	// Get Physical Device - GPU
	uint32_t gpu_count = 0;
	vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
	std::vector<VkPhysicalDevice> physical_devices(gpu_count);
	vkEnumeratePhysicalDevices(instance, &gpu_count, physical_devices.data());
	//physical_device = physical_devices[0];

	if (gpu_count == 0)
	{
		assert(1 && "[!] Vulkan Error: Failed to find a supporting GPU.");
		std::exit(-1);
	}

	for (const auto& device : physical_devices) 
	{
		QueueFamilyIndices indices = findQueueFamilies(device);

		if (indices.hasEntry()) 
		{
			physical_device = device;
			break;
		}
	}

	if (physical_device == VK_NULL_HANDLE) 
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

}


Renderer::QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device)
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
	for (uint32_t i = 0; i < family_count; ++i)
	{
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphics_family_index = i;
			indices.graphicsFamily = i;
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


// Initialize Vulkan Device
void Renderer::createDevice()
{
	float queue_priorities[]{ 1.0f };

	// Allocate Device Queue from Queue Family
	VkDeviceQueueCreateInfo device_queue_create_info {};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueFamilyIndex = graphics_family_index;
	device_queue_create_info.queueCount = 1;
	device_queue_create_info.pQueuePriorities = queue_priorities;

	// Create Device Info
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &device_queue_create_info;
	device_create_info.enabledLayerCount = device_layers.size();
	device_create_info.ppEnabledLayerNames = device_layers.data();
	device_create_info.enabledExtensionCount = device_extensions.size();
	device_create_info.ppEnabledExtensionNames = device_extensions.data();

	result = errorHandler(vkCreateDevice(physical_device, &device_create_info, nullptr, &device));
	if (result != VK_SUCCESS)
	{
		std::cout << ("\n[!] Failed to Create Vulkan Device");
		std::exit(-1);
	}
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
