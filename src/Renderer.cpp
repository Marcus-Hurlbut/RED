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
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandPool();
	createCommandBuffer();
	createSyncObjects();
}


void Renderer::deInitVulkan()
{
	// Destroy Sync objects
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroyFence(device, inFlightFence, nullptr);

	// Destroy Command Pool
	vkDestroyCommandPool(device, commandPool, nullptr);

	// Destroy Frame Buffers
	for (auto framebuffer : swapChainFrameBuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	// Destroy Graphics pipeline
	vkDestroyPipeline(device, graphicsPipeline, nullptr);

	// Destroy Pipeline layout
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr); 

	// Destroy the Render Pass
	vkDestroyRenderPass(device, render_pass, nullptr);

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
	glfwDestroyWindow(window);

	glfwTerminate();
}


void Renderer::eventHandler()
{
	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();
		drawFrame();
	}
	vkDeviceWaitIdle(device);
}


// SDL Window Initializaiton and Creation
void Renderer::createWindow()
{
	// Initialize SDL 
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);

	if (window == nullptr)
	{
		throw std::runtime_error("\n[!] SDL Error: Unable to initilaize SDL window.\n");
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
		throw std::runtime_error("\n[!] Validation Error: layers Requested but NOT Found.");
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
	auto extensions = checkSDLExtensions();
	create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	create_info.ppEnabledExtensionNames = extensions.data();

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
		throw std::runtime_error("[!] Failed to Create a Vulkan Instance.");
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
std::vector<const char*> Renderer::checkSDLExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
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
		throw std::runtime_error("[!] Vulkan Error: Failed to find a supporting GPU.");
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
		std::exit(-1);
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
		throw std::runtime_error("[!] Vulkan Error: Queue family for supporting graphics not found.");
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
		throw std::runtime_error("\n[!] Failed to Create Vulkan Logical Device");
		std::exit(-1);
	}

	// Get Logical Device Queue Handles
	vkGetDeviceQueue(device, queue_family_index, 0, &graphics_queue);
	vkGetDeviceQueue(device, present_family_index, 0, &present_queue);

}


// Initialize Window Surface
void Renderer::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) 
	{
		throw std::runtime_error("[!] Failed to create window surface!");
		std::exit(-1);
	}
}


// Retrieve Swap Chain Property Info
Renderer::SwapChainProperties Renderer::querySwapChainProp(VkPhysicalDevice device)
{
	SwapChainProperties properties{};
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
		glfwGetFramebufferSize(window, &width, &height);


		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		availableProperties.extent.height = actualExtent.height;
		availableProperties.extent.width = actualExtent.width;

		throw std::runtime_error("[!] Resolution Error: Swap Chain Extent is set past the maximum!");
		std::exit(-1);
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
		throw std::runtime_error("[!] Swap Chain Error - Failed to create Swap Chain.");
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
			throw std::runtime_error("[!] Failed to create image views!");
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
		throw std::runtime_error("[!] Shader Module Error - Unable to create Shader module.");
		std::exit(-1);
	}
	return shaderModule;
}


void Renderer::createGraphicsPipeline()
{
	// Get Shader Vertices & Fragment from Buffer
	auto shaderVert = readFile(SHADER_VERT_FILE_DIR);
	auto shaderFrag = readFile(SHADER_FRAG_FILE_DIR);

	// Create Shader Module
	VkShaderModule shaderVertModule = createShaderModule(shaderVert);
	VkShaderModule shaderFragModule = createShaderModule(shaderFrag);

	// Create Shader Vertices Stage
	VkPipelineShaderStageCreateInfo vert_create_info {};
	vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_create_info.module = shaderVertModule;
	vert_create_info.pName = "main";

	// Create Shader Fragment Stage
	VkPipelineShaderStageCreateInfo frag_create_info{};
	frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_create_info.module = shaderFragModule;
	frag_create_info.pName = "main";

	// Create Shader Stages from Fragments & Verticies
	VkPipelineShaderStageCreateInfo stages[] = { vert_create_info, frag_create_info };

	// Create Vertices
	VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
	vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_create_info.vertexBindingDescriptionCount = 0;
	vertex_input_create_info.pVertexAttributeDescriptions = 0;

	VkPipelineInputAssemblyStateCreateInfo assembly_create_info{};
	assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	assembly_create_info.primitiveRestartEnable = VK_FALSE;

	// Create Viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swap_chain_extent.width;
	viewport.height = (float)swap_chain_extent.height;

	// Create Scissor for viewport
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swap_chain_extent;

	// Create Viewport Pipeline
	VkPipelineViewportStateCreateInfo viewport_create_info{};
	viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_create_info.viewportCount = 1;
	viewport_create_info.pViewports = &viewport;
	viewport_create_info.scissorCount = 1;  // Currently only 1 scissor implemented
	viewport_create_info.pScissors = &scissor;

	// Create Rasterizor
	VkPipelineRasterizationStateCreateInfo rasterizer_create_info{};
	rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_create_info.depthClampEnable = VK_FALSE;
	rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_create_info.lineWidth = 1.0f;
	rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer_create_info.depthBiasEnable = VK_FALSE;

	// Create Multisampling
	VkPipelineMultisampleStateCreateInfo multisample_create_info{};
	multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_create_info.sampleShadingEnable = VK_FALSE;
	multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;						// Come back and play around with this value

	// Create Color Blend Attachments
	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	// Create Color Blending Pipeline
	VkPipelineColorBlendStateCreateInfo color_blend_create_info{};
	color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_create_info.logicOpEnable = VK_FALSE;
	color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
	color_blend_create_info.attachmentCount = 1;
	color_blend_create_info.pAttachments = &color_blend_attachment;
	color_blend_create_info.blendConstants[0] = 0.0f;
	color_blend_create_info.blendConstants[1] = 0.0f;
	color_blend_create_info.blendConstants[2] = 0.0f;
	color_blend_create_info.blendConstants[3] = 0.0f;

	// Create Pipeline Layout
	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = 0;
	pipeline_layout_create_info.pushConstantRangeCount = 0;

	// Pipeline layout error handling
	if (errorHandler(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipelineLayout)) != VK_SUCCESS)
	{
		throw std::runtime_error("[!] Failed to create pipeline layout!");
		std::exit(-1);
	}

	// Create Pipeline
	VkGraphicsPipelineCreateInfo pipeline_create_info{};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.stageCount = 2;
	pipeline_create_info.pStages = stages;
	pipeline_create_info.pVertexInputState = &vertex_input_create_info;
	pipeline_create_info.pInputAssemblyState = &assembly_create_info;
	pipeline_create_info.pViewportState = &viewport_create_info;
	pipeline_create_info.pRasterizationState = &rasterizer_create_info;
	pipeline_create_info.pMultisampleState = &multisample_create_info;
	pipeline_create_info.pColorBlendState = &color_blend_create_info;
	pipeline_create_info.layout = pipelineLayout;
	pipeline_create_info.renderPass = render_pass;
	pipeline_create_info.subpass = 0;
	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &graphicsPipeline) != VK_SUCCESS) 
	{
		throw std::runtime_error("[!] Failed to create graphics pipeline!");
		std::exit(-1);
	}

	// Destroy Shader Module 
	vkDestroyShaderModule(device, shaderFragModule, nullptr);
	vkDestroyShaderModule(device, shaderVertModule, nullptr);
}


void Renderer::createRenderPass()
{
	// Color Attachment for Render Pass
	VkAttachmentDescription color_attachment{};
	color_attachment.format = swap_chain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Color Attachment Reference
	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Subpass Description
	VkSubpassDescription subpass_description{};
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &color_attachment_ref;

	// Render Pass Info
	VkRenderPassCreateInfo render_pass_create_info{};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = 1;
	render_pass_create_info.pAttachments = &color_attachment;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass_description;

	// Error Handling
	if (vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass) != VK_SUCCESS)
	{
		throw std::runtime_error("[!] Failed to create Render pass.");
		std::exit(-1);
	}
}

void Renderer::createFrameBuffers()
{
	// Resize container to hold all of the Frame buffers
	swapChainFrameBuffers.resize(swapChainImageViews.size());

	// Iterate through the image views and create framebuffers from them
	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		VkImageView attachments[] =
		{
			swapChainImageViews[i]
		};

		// Create Frame Buffer Info
		VkFramebufferCreateInfo frame_buffer_create_info{};
		frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_create_info.renderPass = render_pass;
		frame_buffer_create_info.attachmentCount = 1;
		frame_buffer_create_info.pAttachments = attachments;
		frame_buffer_create_info.width = swap_chain_extent.width;
		frame_buffer_create_info.height = swap_chain_extent.height;
		frame_buffer_create_info.layers = 1;

		// Error Handling
		if (errorHandler(vkCreateFramebuffer(device, &frame_buffer_create_info, nullptr, &swapChainFrameBuffers[i])) != VK_SUCCESS)
		{
			throw std::runtime_error("[!] Failed to Create Framebuffer.");
			std::exit(-1);
		}
	}
}


void Renderer::createCommandPool()
{
	// Get Queue families
	QueueFamilyIndices queueFamilyIndices = queryQueueFamilies(physical_device);

	// recording a command buffer every frame
	VkCommandPoolCreateInfo pool_create_info{};
	pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_create_info.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

	if (vkCreateCommandPool(device, &pool_create_info, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("[!] Failed to Create Command pool.");
		std::exit(-1);
	}
}


void Renderer::createCommandBuffer()
{
	// Allocate Command Buffer
	VkCommandBufferAllocateInfo command_buffer_alloc_info{};
	command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_alloc_info.commandPool = commandPool;
	command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_alloc_info.commandBufferCount = 1;

	if (errorHandler(vkAllocateCommandBuffers(device, &command_buffer_alloc_info, &commandBuffer)) != VK_SUCCESS)
	{
		throw std::runtime_error("[!] Failed to allocate Command buffers!");
		std::exit(-1);
	}
}


void Renderer::writeCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index)
{
	// Begin recording to command buffer
	VkCommandBufferBeginInfo command_buffer_begin_info{};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = 0; // Optional
	command_buffer_begin_info.pInheritanceInfo = nullptr; // Optional

	if (errorHandler(vkBeginCommandBuffer(commandBuffer, &command_buffer_begin_info))!= VK_SUCCESS)
	{
		throw std::runtime_error("[!] Failed to begin writing to Command Buffer!");
		std::exit(-1);
	}

	// Start the Render passing process
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = render_pass;
	renderPassInfo.framebuffer = swapChainFrameBuffers[image_index];

	// Bind the framebuffer for the swapchain image we want to draw
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swap_chain_extent;

	// Define the size of the render area
	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	// Start render passing
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to record command buffer!");
		std::exit(-1);
	}
}


void Renderer::createSyncObjects()
{
	// Create info from semaphore object
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
		vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create synchronization objects for a frame!");
		std::exit(-1);
	}
}


void Renderer::drawFrame()
{
	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFence);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swap_chain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
	writeCommandBuffer(commandBuffer, imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphics_queue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to submit draw command buffer!");
		std::exit(-1);
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swap_chain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(present_queue, &presentInfo);
}


std::vector<char> Renderer::readFile(const std::string &fileName)
{
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open()) 
	{
		throw std::runtime_error("[!] File Error - failed to open and read in file. ");
		std::exit(-1);
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;;
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

	case VK_SUCCESS:
		break;

	default:
		break;
	};

	return error;
}
