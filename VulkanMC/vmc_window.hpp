#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>	
#include <string>

namespace vmc {
	class VmcWindow {
	public:
		VmcWindow(uint16_t w, uint16_t h, std::string name, void (*windowRefreshCallback)(GLFWwindow* window));
		~VmcWindow();

		// delete copy and assignment constructors to prevent dangling pointers (RAII)
		VmcWindow(const VmcWindow&) = delete;
		VmcWindow& operator=(const VmcWindow&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D getExtent() {
			return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		}
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		void initWindow();

		void (*windowRefreshCallback)(GLFWwindow* window);
	private:
		static void frameBufferResizedCallback(GLFWwindow* window, int width, int height);

		int width;
		int height;
		bool framebufferResized = false;

		GLFWwindow* window;
		std::string windowName;
	};
}
