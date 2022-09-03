#include "vmc_renderer.hpp"

#include <stdexcept>
#include <array>
#include <math.h>
#include <iostream>
namespace vmc {

	VmcRenderer::VmcRenderer(VmcWindow& window, VmcDevice& device) :vmcWindow{ window }, vmcDevice{ device } {
		recreateSwapChain();
		createCommandBuffers();
	}

	VmcRenderer::~VmcRenderer() {
		freeCommandBuffers();
	}

	void VmcRenderer::recreateSwapChain() {
		auto extent = vmcWindow.getExtent();

		// while at least one of the window dimensions are sizeless, the program will pause and wait
		while (extent.width == 0 || extent.height == 0) {
			extent = vmcWindow.getExtent();
			glfwWaitEvents();
		}

		// wait until the current swapchain isnt being used until we create the new one or also scavenge the old one for resources
		vkDeviceWaitIdle(vmcDevice.device());

		if (vmcSwapChain == nullptr) {
			vmcSwapChain = std::make_unique<VmcSwapChain>(vmcDevice, extent);
		}
		else {
			std::shared_ptr<VmcSwapChain> oldSwapChain = std::move(vmcSwapChain);
			vmcSwapChain = std::make_unique<VmcSwapChain>(vmcDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*vmcSwapChain.get())) {
				throw std::runtime_error("Swap chain image format has changed");
			}
		}

		// if render passes are compatible then we don't need to recreate the pipeline
	}

	void VmcRenderer::createCommandBuffers() {
		// MAX_FRAMES_IN_FLIGHT will be 2 or 3 depending on how many frame buffers you use
		commandBuffers.resize(VmcSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		// primary command buffers can be submitted to a queue for execution but cannot be called by other command buffers
		// secondary command buffers are the opposite
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = vmcDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(vmcDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}
	}

	void VmcRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(vmcDevice.device(), vmcDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	VkCommandBuffer VmcRenderer::beginFrame() {
		assert(!isFrameStarted && "Can't call beginFrame while already in progress");

		auto result = vmcSwapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire next swap chain image");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}
		return commandBuffer;
	}

	void VmcRenderer::endFrame() {
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
		}

		auto result = vmcSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vmcWindow.wasWindowResized()) {
			vmcWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % VmcSwapChain::MAX_FRAMES_IN_FLIGHT;
	}
	void VmcRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin renderPass on command buffer from a different frame");

		// A blueprint that tells the graphics pipeline what layout to expect for an output framebuffer + other instructions
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vmcSwapChain->getRenderPass();
		renderPassInfo.framebuffer = vmcSwapChain->getFrameBuffer(currentImageIndex);

		// defines the area where shader loads and stores will take place
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vmcSwapChain->getSwapChainExtent();

		// index 0 is the color attachment, index 1 is the depth attachment
		// this is the background
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// INLINE means that the subsuqent renderpass commands will be embedded in the primary command buffer 
		// no secondary command buffers will be used
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// dynamically set the viewport and put it in the command buffer. 
		// we will always be using the correct window size even if the swapchain changes
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vmcSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(vmcSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, vmcSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}
	void VmcRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't end renderPass on command buffer from a different frame");

		vkCmdEndRenderPass(commandBuffer);
	}
}