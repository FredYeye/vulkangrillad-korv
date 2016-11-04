#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


struct SwapChainSupportDetails
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
		void CreateSwapChain();

		void PrintAvailableExtensions();
		void Destroy();

	private:
		std::vector<const char*> GetRequiredExtensions();
		bool IsDeviceSuitable(VkPhysicalDevice physDevice);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice physDevice);
		bool CheckValidationLayerSupport();
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physDevice);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

		VkInstance instance = 0;
		VkDebugReportCallbackEXT callback = 0;
		VkSurfaceKHR surface = 0;
		std::vector<VkPhysicalDevice> devices;
		VkPhysicalDevice physicalDevice = 0;
		VkDevice device = 0;
		const std::vector<const char*> validationLayers{"VK_LAYER_LUNARG_standard_validation"};
		const std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME}; //glfw requires khr_swapchain
};
