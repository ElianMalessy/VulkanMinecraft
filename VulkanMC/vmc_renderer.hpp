#pragma once

#pragma once

#include "vmc_window.hpp"
#include "vmc_device.hpp"
#include "vmc_swap_chain.hpp"

#include <cassert>
#include <memory>
#include <vector>
#include <functional>

namespace vmc {
	class VmcRenderer {
	public:
		VmcRenderer(VmcWindow& window, VmcDevice& device);
		~VmcRenderer();

		VmcRenderer(const VmcRenderer&) = delete;
		VmcRenderer& operator=(const VmcRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return vmcSwapChain->getRenderPass(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
	private:
		static void windowRefreshCallback(GLFWwindow* window);

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		VmcWindow& vmcWindow;
		VmcDevice& vmcDevice;

		std::unique_ptr<VmcSwapChain> vmcSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex;
		bool isFrameStarted = false;
	};
}
