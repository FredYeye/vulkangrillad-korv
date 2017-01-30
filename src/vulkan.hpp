#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Vulkan
{
	public:
		void CreateInstance();
		void SetupDebugCallback();
		void CreateSurface(GLFWwindow* &window);
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapchain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSemaphores();

		void DrawFrame();

		void PrintAvailableExtensions();
		void Destroy();

		bool verbose = false;

	private:
		std::vector<const char*> GetRequiredExtensions();
		bool IsDeviceSuitable(VkPhysicalDevice physDevice);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice physDevice);
		bool CheckValidationLayerSupport();
		SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physDevice);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
		int FindQueueFamily(VkPhysicalDevice physDevice);
		void CreateShaderModule(const std::vector<uint32_t> &code, VkShaderModule &module);

		const std::vector<const char*> validationLayers{"VK_LAYER_LUNARG_standard_validation"};
		const std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;
		//
		std::vector<VkFramebuffer> swapchainFramebuffers;
		std::vector<VkCommandBuffer> commandBuffers;

		VkQueue graphicsQueue, presentQueue;
		VkFormat swapchainImageFormat;
		VkExtent2D swapchainExtent;

		VkPhysicalDevice physicalDevice = 0;
		//
		VkInstance instance = 0;
		VkDevice device = 0;
		VkSurfaceKHR surface = 0;
		VkSwapchainKHR swapchain = 0;
		VkPipelineLayout pipelineLayout = 0;
		VkRenderPass renderPass = 0;
		VkPipeline graphicsPipeline = 0;
		VkCommandPool commandPool = 0;
		VkSemaphore imageAvailable = 0, renderFinished = 0;
		VkDebugReportCallbackEXT callback = 0;
};
