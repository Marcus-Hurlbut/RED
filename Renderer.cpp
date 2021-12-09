#include "Renderer.h"

Renderer::Renderer()
{
	initWindow();
	initInstance();
	initDebug();
	setupDevice();
	initDevice();
}

Renderer::~Renderer()
{
	deleteDevice();
	deleteDebug();
	deleteInstance();
	deleteWindow();
}

void Renderer::initWindow()
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

void Renderer::deleteWindow()
{
	SDL_DestroyWindow(window);
}


// Create a Vulkan Instance
void Renderer::initInstance()
{
	// Extend SDL window interface to Vulkan
	uint32_t extension_count = 0;
	SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);
	instance_extensions.resize(extension_count);
	SDL_Vulkan_GetInstanceExtensions(window, &extension_count, instance_extensions.data());

	// Application Information
	VkApplicationInfo application {};
	application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application.pApplicationName = "Vulkan Renderer Prototype";
	application.apiVersion = VK_API_VERSION_1_0;
	application.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	application.pEngineName = "No Engine";

	// Instantiate Vulkan from app info
	VkInstanceCreateInfo instance_create {};
	instance_create.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create.pApplicationInfo = &application;
	instance_create.enabledLayerCount = layers.size();
	instance_create.ppEnabledLayerNames = layers.data();
	instance_create.enabledExtensionCount = instance_extensions.size();
	instance_create.ppEnabledExtensionNames = instance_extensions.data();
	//instance_create.pNext = &debug_callback;

	errorHandler(vkCreateInstance(&instance_create, nullptr, &instance));
}


// Destroy Vulkan Instance
void Renderer::deleteInstance()
{
	vkDestroyInstance(instance, nullptr);
	instance = nullptr;
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

	if (!found)
	{
		assert(1 && "[!] Vulkan Error: Queue family for supporting graphics not found.");
		std::exit(-1);
	}

	// Enumerate Instance Layer Properties
	uint32_t instance_layer_count = 0;
	std::vector<VkLayerProperties> instance_layer_properties;
	vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
	instance_layer_properties.resize(instance_layer_count);
	vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layer_properties.data());

	// Enumerate Device Layer Properties
	uint32_t device_layer_count = 0;
	std::vector<VkLayerProperties> device_layer_properties;
	vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, nullptr);
	device_layer_properties.resize(device_layer_count);
	vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, device_layer_properties.data());

	// Print all Instance Layers
	std::cout << "Instance Layers:\n";
	for (auto& i : instance_layer_properties)
	{
		std::cout << "\n " << i.layerName << "\t\t | " << i.description;
	};

	// Print all Device Layers
	std::cout << "Device Layers:\n";
	for (auto& i : device_layer_properties)
	{
		std::cout << "\n " << i.layerName << "\t\t | " << i.description;
	};
}


// Initialize Vulkan Device
void Renderer::initDevice()
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

	errorHandler(vkCreateDevice(gpu, &device_create_info, nullptr, &device));

}

// Destroy Vulkan Device 
void Renderer::deleteDevice()
{
	vkDestroyDevice(device, nullptr);
	device = VK_NULL_HANDLE;
}

// Vulkan Debug Callback Function
static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type, uint64_t src_obj,
													size_t loc, int32_t msg_code, const char* layer_prefix, const char* msg, void* user_data)
{
	std::cout << msg << std::endl;
	return false;
}


// Set up Debugger pointer
PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = nullptr;		// Debugger report create ptr

// Initialize Debugger
void Renderer::initDebug()
{
	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) SDL_Vulkan_GetVkGetInstanceProcAddr();

	if (nullptr == fvkCreateDebugReportCallbackEXT)
	{
		assert(1 && "[!] Vulkan Error: Cannot fetch debug function pointers.");
		std::exit(-1);
	}

	VkDebugReportCallbackCreateInfoEXT debug_callback{};
	debug_callback.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debug_callback.pfnCallback = VulkanDebugCallback;
	debug_callback.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;

	//fvkCreateDebugReportCallbackEXT(instance, &debug_callback , 0, &debug_report);
}

// Deinitialize Debugger
void Renderer::deleteDebug()
{
	//fvkDestroyDebugReportCallbackEXT(instance, debug_report, nullptr);
	debug_report = VK_NULL_HANDLE;
}




// Handle Vulkan Result Errors
void Renderer::errorHandler(VkResult error)
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

	/*
	// Provided by VK_KHR_maintenance1
	VK_ERROR_OUT_OF_POOL_MEMORY_KHR = VK_ERROR_OUT_OF_POOL_MEMORY,

	// Provided by VK_KHR_external_memory
	VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR = VK_ERROR_INVALID_EXTERNAL_HANDLE,

	// Provided by VK_EXT_descriptor_indexing
	VK_ERROR_FRAGMENTATION_EXT = VK_ERROR_FRAGMENTATION,

	// Provided by VK_EXT_buffer_device_address
	VK_ERROR_INVALID_DEVICE_ADDRESS_EXT = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,

	// Provided by VK_KHR_buffer_device_address
	VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,

	// Provided by VK_EXT_pipeline_creation_cache_control
	VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT = VK_PIPELINE_COMPILE_REQUIRED_EXT,
	*/
}