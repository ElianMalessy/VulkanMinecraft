#include "vmc_window.hpp"

#include <stdexcept>
#include <iostream>
namespace vmc {
	VmcWindow::VmcWindow(uint16_t w, uint16_t h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	VmcWindow::~VmcWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void VmcWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback);
		//glfwSetWindowRefreshCallback(window, windowRefreshCallback);
	}

	void VmcWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	void VmcWindow::frameBufferResizedCallback(GLFWwindow* window, int width, int height) {
		VmcWindow* vmcWindow = reinterpret_cast<VmcWindow*>(glfwGetWindowUserPointer(window));
		vmcWindow->framebufferResized = true;
		vmcWindow->width = width;
		vmcWindow->height = height;
	}
}