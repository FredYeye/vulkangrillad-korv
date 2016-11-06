#include <iostream>
#include <array>
#include <vector>

#include "vulkan.hpp"
#include "file.hpp"


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


void Vulkan::CreateImageViews()
{
	swapchainImageViews.resize(swapchainImages.size());

	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = swapchainImageFormat;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	for(int x = 0; x < swapchainImages.size(); ++x)
	{
		createInfo.image = swapchainImages[x];
		vkCreateImageView(device, &createInfo, 0, &swapchainImageViews[x]);
	}
}


void Vulkan::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;

	vkCreateRenderPass(device, &renderPassInfo, 0, &renderPass);
}


void Vulkan::CreateGraphicsPipeline()
{
	std::vector<uint8_t> vertShaderCode = FileToU8Vec("bin/vert.spv");
	std::vector<uint8_t> fragShaderCode = FileToU8Vec("bin/frag.spv");

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	CreateShaderModule(vertShaderCode, vertShaderModule);
	CreateShaderModule(fragShaderCode, fragShaderModule);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{vertShaderStageInfo, fragShaderStageInfo};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = 0;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = float(swapchainExtent.width);
	viewport.height = float(swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = 0;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;

	vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &pipelineLayout);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = 0;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = 0;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &graphicsPipeline);

	vkDestroyShaderModule(device, vertShaderModule, 0);
	vkDestroyShaderModule(device, fragShaderModule, 0);
}


void Vulkan::CreateFramebuffers()
{
	swapchainFramebuffers.resize(swapchainImageViews.size());

	for(int x = 0; x < swapchainImageViews.size(); ++x)
	{
		std::array<VkImageView, 1> attachments{swapchainImageViews[x]};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapchainExtent.width;
		framebufferInfo.height = swapchainExtent.height;
		framebufferInfo.layers = 1;

		vkCreateFramebuffer(device, &framebufferInfo, 0, &swapchainFramebuffers[x]);
	}
}


void Vulkan::CreateCommandPool()
{
	int index = FindQueueFamily(physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = index;
	poolInfo.flags = 0;

	vkCreateCommandPool(device, &poolInfo, 0, &commandPool);
}


void Vulkan::CreateCommandBuffers()
{
	commandBuffers.resize(swapchainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = commandBuffers.size();

	vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());

	for(int x = 0; x < commandBuffers.size(); ++x)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = 0;

		vkBeginCommandBuffer(commandBuffers[x], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapchainFramebuffers[x];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = swapchainExtent;
		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers[x], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[x], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		vkCmdDraw(commandBuffers[x], 3, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffers[x]);

		vkEndCommandBuffer(commandBuffers[x]);
	}
}


void Vulkan::CreateSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(device, &semaphoreInfo, 0, &imageAvailable);
	vkCreateSemaphore(device, &semaphoreInfo, 0, &renderFinished);
}


void Vulkan::DrawFrame()
{
	
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

	if(imageAvailable)
	{
		vkDestroySemaphore(device, imageAvailable, 0);
	}
	if(renderFinished)
	{
		vkDestroySemaphore(device, renderFinished, 0);
	}
	if(commandPool)
	{
		vkDestroyCommandPool(device, commandPool, 0);
	}
	for(auto &v : swapchainFramebuffers)
	{
		if(v)
		{
			vkDestroyFramebuffer(device, v, 0);
		}
	}
	if(graphicsPipeline)
	{
		vkDestroyPipeline(device, graphicsPipeline, 0);
	}
	if(renderPass)
	{
		vkDestroyRenderPass(device, renderPass, 0);
	}
	if(pipelineLayout)
	{
		vkDestroyPipelineLayout(device, pipelineLayout, 0);
	}
	for(auto &v : swapchainImageViews)
	{
		if(v)
		{
			vkDestroyImageView(device, v, 0);
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


void Vulkan::CreateShaderModule(const std::vector<uint8_t> &code, VkShaderModule &module)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = (uint32_t*)code.data();

	vkCreateShaderModule(device, &createInfo, 0, &module);
}
