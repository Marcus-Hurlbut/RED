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
	createSDLWindow();
	createInstance();
	createDebug();
	setupDevice();
	createDevice();
}

void Renderer::deInitVulkan()
{
	// Destroy device
	vkDestroyDevice(device, nullptr);
	device = VK_NULL_HANDLE;

	// Destroy Debugger Report
	//fvkDestroyDebugReportCallbackEXT(instance, debug_report, nullptr);
	debug_report = VK_NULL_HANDLE;

	// Destroy Instance
	vkDestroyInstance(instance, nullptr);
	instance = nullptr;

	// Destroy SDL Window
	SDL_DestroyWindow(window);
}

// SDL Window Initializaiton and Creation
void Renderer::createSDLWindow()
{
	// Initialize SDL 
	SDL_Init(SDL_INIT_VIDEO);
	window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
	window = SDL_CreateWindow("Vulkan Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);

	if (window == nullptr)
	{
		assert(1 && "[!] SDL Error: Unable to initilaize SDL window.");
		std::exit(-1);
	}
}


// Create a Vulkan Instance
void Renderer::createInstance()
{
	// Extend SDL extensions to Vulkan
	uint32_t extension_count = 0;
	SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);
	SDL_extensions.resize(extension_count);
	SDL_Vulkan_GetInstanceExtensions(window, &extension_count, SDL_extensions.data());

	// Set Application Info
	VkApplicationInfo application {};
	application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application.pApplicationName = "Vulkan Renderer Prototype";
	application.apiVersion = VK_API_VERSION_1_0;
	application.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	application.pEngineName = "No Engine";

	// Instantiate Vulkan from Application Info and SDL Extensions
	VkInstanceCreateInfo instance_create {};
	instance_create.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create.pApplicationInfo = &application;
	instance_create.enabledLayerCount = layers.size();
	instance_create.ppEnabledLayerNames = layers.data();
	instance_create.enabledExtensionCount = SDL_extensions.size();
	instance_create.ppEnabledExtensionNames = SDL_extensions.data();

	// Instance Error Handling
	if (errorHandler(vkCreateInstance(&instance_create, nullptr, &instance)) != VK_SUCCESS)
	{
		throw std::runtime_error("[!] Failed to Create a Vulkan Instance.");
	}

	// Check Instance Extensions support
	uint32_t instance_extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
	std::vector <VkExtensionProperties> extensions(instance_extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, extensions.data());

	// Print Supported Instance Extensions
	std::cout << "\nInstance Extensions:\n";
	for (const auto& extension : extensions)
	{
		std::cout << '\t' << extension.extensionName << '\n';
	}

	// Check Instance Layer Properties
	uint32_t layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties> layer_properties(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data());

	// Print all Instance Layers
	std::cout << "Instance Layers:\n";
	for (auto& i : layer_properties)
	{
		std::cout << "\n " << i.layerName << "\t\t | " << i.description;
	};
}



// Setup physical device & queue families for initialization
void Renderer::setupDevice()
{
	// Initialize Physical Device - GPU
	uint32_t gpu_count = 0;
	std::vector<VkPhysicalDevice> gpu_list;
	vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
	gpu_list.resize(gpu_count);
	vkEnumeratePhysicalDevices(instance, &gpu_count, gpu_list.data());
	gpu = gpu_list[0];

	if (gpu_count == 0)
	{
		assert(1 && "[!] Vulkan Error: Failed to find a supporting GPU.");
		std::exit(-1);
	}

	// Initialize Queue Family
	uint32_t family_count = 0;
	std::vector<VkQueueFamilyProperties> queue_families;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, nullptr);
	queue_families.resize(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, queue_families.data());

	// Find a Supporting Queue Family
	bool found = false;
	for (uint32_t i = 0; i < family_count; ++i)
	{
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			found = true;
			graphics_family_index = i;
		}
	}

	// Error Handling for Queue Family
	if (!found)
	{
		assert(1 && "[!] Vulkan Error: Queue family for supporting graphics not found.");
		std::exit(-1);
	}

	// Enumerate Device Layer Properties
	uint32_t device_layer_count = 0;
	std::vector<VkLayerProperties> device_layer_properties;
	vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, nullptr);
	device_layer_properties.resize(device_layer_count);
	vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, device_layer_properties.data());


	// Print all Device Layers
	std::cout << "\nDevice Layers:\n";
	for (auto& i : device_layer_properties)
	{
		std::cout << "\n " << i.layerName << "\t\t | " << i.description;
	};
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
	device_create_info.enabledLayerCount = layers.size();
	device_create_info.ppEnabledLayerNames = layers.data();
	device_create_info.enabledExtensionCount = device_extensions.size();
	device_create_info.ppEnabledExtensionNames = device_extensions.data();

	result = errorHandler(vkCreateDevice(gpu, &device_create_info, nullptr, &device));
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("\n[!] Failed to Create Vulkan Device");
	}
}


// Vulkan Debug Callback Function
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}


// Set up Debugger pointer
PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = nullptr;		// Debugger report create ptr

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) 
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

// Initialize Debugger
void Renderer::createDebug()
{
	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) SDL_Vulkan_GetVkGetInstanceProcAddr();

	if (nullptr == fvkCreateDebugReportCallbackEXT)
	{
		assert(1 && "[!] Vulkan Error: Cannot fetch debug function pointers.");
		std::exit(-1);
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}


// Handle Vulkan Result Errors
VkResult Renderer::errorHandler(VkResult error)
{
	switch (error)
	{
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
		break;

	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
		break;

	case VK_ERROR_INITIALIZATION_FAILED:
		std::cout << "VK_ERROR_INITIALIZATION_FAILED" << std::endl;
		break;

	case VK_ERROR_DEVICE_LOST:
		std::cout << "VK_ERROR_DEVICE_LOST" << std::endl;
		break;

	case VK_ERROR_MEMORY_MAP_FAILED:
		std::cout << "VK_ERROR_MEMORY_MAP_FAILED" << std::endl;
		break;

	case VK_ERROR_LAYER_NOT_PRESENT:
		std::cout << "VK_ERROR_LAYER_NOT_PRESENT" << std::endl;
		break;

	case VK_ERROR_EXTENSION_NOT_PRESENT:
		std::cout << "VK_ERROR_EXTENSION_NOT_PRESENT" << std::endl;
		break;

	case VK_ERROR_FEATURE_NOT_PRESENT:
		std::cout << "VK_ERROR_FEATURE_NOT_PRESENT" << std::endl;
		break;

	case VK_ERROR_INCOMPATIBLE_DRIVER:
		std::cout << "VK_ERROR_INCOMPATIBLE_DRIVER" << std::endl;
		break;

	case VK_ERROR_TOO_MANY_OBJECTS:
		std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
		break;

	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		std::cout << "VK_ERROR_FORMAT_NOT_SUPPORTED" << std::endl;
		break;

	case VK_ERROR_FRAGMENTED_POOL:
		std::cout << "VK_ERROR_FRAGMENTED_POOL" << std::endl;
		break;

	case VK_ERROR_UNKNOWN:
		std::cout << "VK_ERROR_UNKNOWN" << std::endl;
		break;

	case VK_ERROR_OUT_OF_POOL_MEMORY:
		std::cout << "VK_ERROR_OUT_OF_POOL_MEMORY" << std::endl;				// Provided by VK_VERSION_1_1
		break;

	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		std::cout << "VK_ERROR_INVALID_EXTERNAL_HANDLE" << std::endl;			// Provided by VK_VERSION_1_1
		break;

	case VK_ERROR_FRAGMENTATION:
		std::cout << "VK_ERROR_FRAGMENTATION" << std::endl;						// Provided by VK_VERSION_1_2
		break;

	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
		std::cout << "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS" << std::endl;	// Provided by VK_VERSION_1_2
		break;

	case VK_ERROR_SURFACE_LOST_KHR:
		std::cout << "VK_ERROR_SURFACE_LOST_KHR" << std::endl;					// Provided by VK_KHR_surface
		break;

	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		std::cout << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" << std::endl;			// Provided by VK_KHR_surface
		break;

	case VK_SUBOPTIMAL_KHR:
		std::cout << "VK_SUBOPTIMAL_KHR" << std::endl;							// Provided by VK_KHR_swapchain
		break;

	case VK_ERROR_OUT_OF_DATE_KHR:
		std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;					// Provided by VK_KHR_swapchain
		break;

	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		std::cout << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" << std::endl;			// Provided by VK_KHR_display_swapchain
		break;

	case VK_ERROR_VALIDATION_FAILED_EXT:
		std::cout << "VK_ERROR_VALIDATION_FAILED_EXT " << std::endl;			// Provided by VK_EXT_debug_report
		break;

	case VK_ERROR_INVALID_SHADER_NV:
		std::cout << "VK_ERROR_INVALID_SHADER_NV" << std::endl;					// Provided by VK_NV_glsl_shader
		break;

	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		std::cout << "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT" << std::endl;	// Provided by VK_EXT_image_drm_format_modifier
		break;

	case VK_ERROR_NOT_PERMITTED_EXT:
		std::cout << "VK_ERROR_NOT_PERMITTED_EXT" << std::endl;					// Provided by VK_EXT_global_priority
		break;

	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		std::cout << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" << std::endl;		// Provided by VK_EXT_full_screen_exclusive
		break;

	case VK_THREAD_IDLE_KHR:
		std::cout << "VK_THREAD_IDLE_KHR" << std::endl;							// Provided by VK_KHR_deferred_host_operations
		break;

	case VK_THREAD_DONE_KHR:
		std::cout << "VK_THREAD_DONE_KHR" << std::endl;							// Provided by VK_KHR_deferred_host_operations
		break;

	case VK_OPERATION_DEFERRED_KHR:
		std::cout << "VK_OPERATION_DEFERRED_KHR" << std::endl;					// Provided by VK_KHR_deferred_host_operations
		break;

	case VK_OPERATION_NOT_DEFERRED_KHR:
		std::cout << "VK_OPERATION_NOT_DEFERRED_KHR" << std::endl;				// Provided by VK_KHR_deferred_host_operations
		break;

	case VK_PIPELINE_COMPILE_REQUIRED_EXT:
		std::cout << "VK_OPERATION_NOT_DEFERRED_KHR" << std::endl;				// Provided by VK_EXT_pipeline_creation_cache_control
		break;

	default:
		break;
	};

	return error;
}
