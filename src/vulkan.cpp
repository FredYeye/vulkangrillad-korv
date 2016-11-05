#include <iostream>
#include <array>
#include <vector>

#include "vulkan.hpp"


#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* layerPrefix,
    const char* msg,
    void* userData)
{
	std::cout << "Validation layer: " << msg << "\n";
	return VK_FALSE;
}


void Vulkan::CreateInstance()
{
	//
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	//
	const std::vector<const char*> extensions = GetRequiredExtensions();

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledLayerCount = 0;
	instanceCreateInfo.enabledExtensionCount = extensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

	if(enableValidationLayers)
	{
		if(CheckValidationLayerSupport())
		{
			instanceCreateInfo.enabledLayerCount = validationLayers.size();
			instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
			std::cout << "Validation layers activated.\n\n";
		}
		else
		{
			std::cout << "Validation layers requested but unavailable!\n";
		}
	}

	//
	VkResult result = vkCreateInstance(&instanceCreateInfo, 0, &instance);
	if(result != VK_SUCCESS)
	{
		std::cout << "vkCreateInstance failed\n";
		// exit(-1); //free things with function later
	}
}

void Vulkan::SetupDebugCallback()
{
	if(enableValidationLayers)
	{
		VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debugCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		debugCreateInfo.pfnCallback = debugCallback;

		PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) glfwGetInstanceProcAddress(instance, "vkCreateDebugReportCallbackEXT");
		if(vkCreateDebugReportCallbackEXT)
		{
			VkResult asd = vkCreateDebugReportCallbackEXT(instance, &debugCreateInfo, 0, &callback);
		}
		else
		{
			std::cout << "Loading vkCreateDebugReportCallbackEXT failed!\n";
		}
	}
}


void Vulkan::CreateSurface(GLFWwindow* &window)
{
	VkResult err = glfwCreateWindowSurface(instance, window, 0, &surface);
	if(err)
	{
		std::cout << "Window surface creation failed";
	}
}


void Vulkan::PickPhysicalDevice()
{
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(instance, &deviceCount, 0);
	if(!deviceCount)
	{
		std::cout << "Found no GPU with Vulkan support!\n";
		// exit(-1);
	}
	devices.resize(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	if(IsDeviceSuitable(devices[0]))
	{
		physicalDevice = devices[0];
	}
	else
	{
		std::cout << "No suitable GPUs found!\n";
		// exit(-1);
	}
}


void Vulkan::CreateLogicalDevice()
{
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, 0);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int index = FindQueueFamily(physicalDevice);

	//
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = index;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	//
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if(enableValidationLayers)
	{
		deviceCreateInfo.enabledLayerCount = validationLayers.size();
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}

	vkCreateDevice(physicalDevice, &deviceCreateInfo, 0, &device);

	VkQueue graphicsQueue;
	vkGetDeviceQueue(device, index, 0, &graphicsQueue);


	//
	VkQueue presentQueue;
	vkGetDeviceQueue(device, index, 0, &presentQueue);
}


void Vulkan::CreateSwapchain()
{
	SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapchainSupport.capabilities);

	// std::cout << "min: " << swapchainSupport.capabilities.minImageCount
			  // << " max: " << swapchainSupport.capabilities.maxImageCount << "\n";
	uint32_t imageCount = swapchainSupport.capabilities.minImageCount;
	if(swapchainSupport.capabilities.maxImageCount > swapchainSupport.capabilities.minImageCount)
	{
		++imageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	int index = FindQueueFamily(physicalDevice);
	// if (indices.graphicsFamily != indices.presentFamily) //use this later
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = 0;

	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	vkCreateSwapchainKHR(device, &createInfo, 0, &swapchain);

	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, 0);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
	swapchainImageFormat = surfaceFormat.format;
	swapchainExtent = extent;
}


void Vulkan::PrintAvailableExtensions()
{
	uint32_t availableExtensionCount;
	vkEnumerateInstanceExtensionProperties(0, &availableExtensionCount, 0);
	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	vkEnumerateInstanceExtensionProperties(0, &availableExtensionCount, availableExtensions.data());

	std::cout << "Available extensions:\n";
	for(const auto &v : availableExtensions)
	{
		std::cout << "  " << v.extensionName << "\n";
	}
	std::cout << "\n";
}


void Vulkan::Destroy()
{
	if(enableValidationLayers)
	{
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = PFN_vkDestroyDebugReportCallbackEXT(glfwGetInstanceProcAddress(instance, "vkDestroyDebugReportCallbackEXT"));
		if(vkDestroyDebugReportCallbackEXT)
		{
			vkDestroyDebugReportCallbackEXT(instance, callback, 0);
		}
		else
		{
			std::cout << "Loading vkDestroyDebugReportCallbackEXT failed!\n";
		}
	}

	if(swapchain)
	{
		vkDestroySwapchainKHR(device, swapchain, 0);
	}
	if(surface)
	{
		vkDestroySurfaceKHR(instance, surface, 0);
	}
	if(device)
	{
		vkDestroyDevice(device, 0);
	}
	if(instance)
	{
		vkDestroyInstance(instance, 0);
	}
}


std::vector<const char*> Vulkan::GetRequiredExtensions()
{
	std::vector<const char*> extensions;

	uint32_t glfwExtensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for(uint32_t x = 0; x < glfwExtensionCount; ++x)
	{
		extensions.push_back(glfwExtensions[x]);
	}

	if(enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}


bool Vulkan::IsDeviceSuitable(VkPhysicalDevice physDevice)
{
	// for(const auto &v : devices)
	// {
		// VkPhysicalDeviceProperties deviceProperties;
		// vkGetPhysicalDeviceProperties(v, &deviceProperties);
		// VkPhysicalDeviceFeatures deviceFeatures;
		// vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

		// if(deviceFeatures.geometryShader)
		// {
			// std::cout << "suitability test";
		// }
	// }

	int suitableQueue = FindQueueFamily(physDevice);

	bool extensionsSupported = CheckDeviceExtensionSupport(physDevice);
	bool swapchainGood = false;
	if(extensionsSupported)
	{
		SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(physDevice);
		swapchainGood = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
	}

	return suitableQueue != -1 && extensionsSupported && swapchainGood;
}


bool Vulkan::CheckDeviceExtensionSupport(VkPhysicalDevice physDevice)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physDevice, 0, &extensionCount, 0);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physDevice, 0, &extensionCount, availableExtensions.data());

	bool extensionSupport = true;
	for(const auto &v : deviceExtensions)
	{
		bool extensionFound = false;
		for(const auto &w : availableExtensions)
		{
			if(std::string(v) == std::string(w.extensionName))
			{
				extensionFound = true;
				break;
			}
		}
		if(extensionFound == false)
		{
			extensionSupport = false;
			break;
		}
	}

	return extensionSupport;
}


bool Vulkan::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, 0);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for(const auto &v : availableLayers)
	{
		if(std::string(v.layerName) == std::string(validationLayers[0]))
		{
			return true;
		}
	}

	return false;
}


SwapchainSupportDetails Vulkan::QuerySwapchainSupport(VkPhysicalDevice physDevice)
{
	SwapchainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, 0);
	if(formatCount)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, 0);
	if(presentModeCount)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


VkSurfaceFormatKHR Vulkan::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	for(const auto &v : availableFormats)
	{
		if(v.format == VK_FORMAT_B8G8R8A8_UNORM && v.colorSpace == false)
		{
			return v;
		}
	}

	std::cout << "i don't know what format this is... format:"
			  << availableFormats[0].format << " colorspace:" << availableFormats[0].colorSpace << "\n";
	return availableFormats[0];	
}


VkPresentModeKHR Vulkan::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	// const std::array<std::string, 4> presentModeNames
	// {
		// "VK_PRESENT_MODE_IMMEDIATE_KHR", "VK_PRESENT_MODE_MAILBOX_KHR",
		// "VK_PRESENT_MODE_FIFO_KHR", "VK_PRESENT_MODE_FIFO_RELAXED_KHR"
	// };
	// std::cout << "presentation engine modes:\n";
	// for(const auto &v : availablePresentModes)
	// {
		// std::cout << "  " << presentModeNames[v] << "\n";
	// }

	return VK_PRESENT_MODE_FIFO_KHR;
}


VkExtent2D Vulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	// std::cout << capabilities.currentExtent.width << " " << capabilities.currentExtent.height << "\n";
	// std::cout << capabilities.maxImageExtent.width << " " << capabilities.maxImageExtent.height << "\n";
	if(capabilities.currentExtent.width != 0xFFFFFFFF)
	{
		return capabilities.currentExtent;
	}
	else
	{
		return {800,600};
	}
}

int Vulkan::FindQueueFamily(VkPhysicalDevice physDevice)
{
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, 0);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, queueFamilies.data());

	int graphicsFamily = -1;
	// int presentFamily = -1; //find a queue with both graphics and present for now
	for(int x = 0; x < queueFamilyCount; ++x)
	{
		VkBool32 presentSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, x, surface, &presentSupport);
		if(queueFamilies[x].queueCount > 0 && queueFamilies[x].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport)
		{
			graphicsFamily = x;
			break;
		}
	}

	return graphicsFamily;
}