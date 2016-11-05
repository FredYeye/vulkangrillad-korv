#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include "main.hpp"
#include "vulkan.hpp"


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(800, 600, "test", 0, 0);

	Vulkan vulkan;
	// vulkan.PrintAvailableExtensions();
	vulkan.CreateInstance();
	vulkan.SetupDebugCallback();
	vulkan.CreateSurface(window);
	vulkan.PickPhysicalDevice();
	vulkan.CreateLogicalDevice();
	vulkan.CreateSwapchain();

	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	vulkan.Destroy();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
