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

	int graphicsFamily = -1;
	// int presentFamily = -1; //find a queue with both graphics and present for now
	for(int x = 0; x < queueFamilyCount; ++x)
	{
		VkBool32 presentSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, x, surface, &presentSupport);
		if(queueFamilies[x].queueCount > 0 && queueFamilies[x].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport)
		{
			graphicsFamily = x;
			break;
		}
	}
	if(graphicsFamily == -1)
	{
		std::cout << "Didn't find a queue family with graphics operations and presentation support!\n";
	}

	//
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = graphicsFamily;
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
	vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);


	//
	VkQueue presentQueue;
	vkGetDeviceQueue(device, graphicsFamily, 0, &presentQueue);
}


void Vulkan::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);

	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
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

	bool extensionsSupported = CheckDeviceExtensionSupport(physDevice);
	bool swapChainGood = false;
	if(extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physDevice);
		swapChainGood = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return extensionsSupported && swapChainGood;
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


SwapChainSupportDetails Vulkan::QuerySwapChainSupport(VkPhysicalDevice physDevice)
{
	SwapChainSupportDetails details;
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
	std::array<std::string, 4> presentModeNames
	{
		"VK_PRESENT_MODE_IMMEDIATE_KHR", "VK_PRESENT_MODE_MAILBOX_KHR",
		"VK_PRESENT_MODE_FIFO_KHR", "VK_PRESENT_MODE_FIFO_RELAXED_KHR"
	};

	std::cout << "presentation engine modes:\n";
	for(const auto &v : availablePresentModes)
	{
		std::cout << "  " << presentModeNames[v] << "\n";
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}
