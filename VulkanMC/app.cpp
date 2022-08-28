#include "app.hpp"

#include <stdexcept>
#include <array>
#include <math.h>
#include <iostream>
namespace vmc {
	App* app;

	App::App() {
		loadModels();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
		app = this;
	}

	App::~App() {
		vkDestroyPipelineLayout(vmcDevice.device(), pipelineLayout, nullptr);
	}

	void App::run() {
		while (!vmcWindow.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}
		// cpu will wait until all gpu operations have been completed
		vkDeviceWaitIdle(vmcDevice.device());
	}

	void App::windowRefreshCallback(GLFWwindow* window) {
		app->drawFrame();
	}

	inline float height(float sidelength) {
		return static_cast<float>(sidelength * sqrt(3) / 2);
	}
	void sterpinsky(glm::vec2 top, glm::vec2 left, glm::vec2 right, int depth, std::vector<VmcModel::Vertex>& vertices) {
		if (depth <= 0) {
			vertices.push_back({ top, {1.0f, 1.0f, 0.0f } });
			vertices.push_back({ right, {1.0f, 0.0f, 0.0f } });
			vertices.push_back({ left, {1.0f, 0.0f, 1.0f } });
		}
		else {

			glm::vec2 topLeft = 0.5f * (left + top);
			glm::vec2 mid = 0.5f * (left + right);
			glm::vec2 topRight = 0.5f * (right + top);


			sterpinsky(topLeft, left, mid, depth - 1, vertices);
			sterpinsky(topRight, mid, right, depth - 1, vertices);
			sterpinsky(top, topLeft, topRight, depth - 1, vertices);
		}
	}
	constexpr float sidelength = 0.9f;
	constexpr float midpoint = 0.0f;
	void App::loadModels() {
		glm::vec2 top = { midpoint, -height(sidelength) };
		glm::vec2 left = { midpoint - sidelength, height(sidelength) };
		glm::vec2 right = { midpoint + sidelength, height(sidelength) };
		std::vector<VmcModel::Vertex> vertices;

		sterpinsky(top, left, right, 3, vertices);

		vmcModel = std::make_unique<VmcModel>(vmcDevice, vertices);
	}

	void App::createPipelineLayout() {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(vmcDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void App::createPipeline() {
		PipelineConfigInfo pipelineConfig{};
		VmcPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = vmcSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		vmcPipeline = std::make_unique<VmcPipeline>(vmcDevice, "default.vert.spv", "default.frag.spv", pipelineConfig);
	}

	void App::recreateSwapChain() {
		VkExtent2D extent = vmcWindow.getExtent();

		// while at least one of the window dimensions are sizeless, the program will pause and wait
		while (extent.width == 0 || extent.height == 0) {
			extent = vmcWindow.getExtent();
			glfwWaitEvents();
		}

		// wait until the current swapchain isnt being used until we create the new one
		vkDeviceWaitIdle(vmcDevice.device());
		vmcSwapChain = std::make_unique<VmcSwapChain>(vmcDevice, extent);
		createPipeline();
	}

	void App::createCommandBuffers() {
		// imagecount will be 2 or 3 depending on how many frame buffers you use
		commandBuffers.resize(vmcSwapChain->imageCount());

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

	void App::recordCommandBuffer(int imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vmcSwapChain->getRenderPass();
		renderPassInfo.framebuffer = vmcSwapChain->getFrameBuffer(imageIndex);

		// defines the area where shader loads and stores will take place
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vmcSwapChain->getSwapChainExtent();

		// index 0 is the color attachment, index 1 is the depth attachment
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// INLINE means that the subsuqent renderpass commands will be embedded in the primary command buffer 
		// no secondary command buffers will be used
		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// draw 3 vertices and 1 instance. An instance can be reused to draw copies with the same vertex data
		vmcPipeline->bind(commandBuffers[imageIndex]);
		vmcModel->bind(commandBuffers[imageIndex]);
		vmcModel->draw(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
		}

	}
	void App::drawFrame() {
		uint32_t imageIndex;
		auto result = vmcSwapChain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire next swap chain image");
		}

		recordCommandBuffer(imageIndex);
		result = vmcSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vmcWindow.wasWindowResized()) {
			vmcWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}
	}

}