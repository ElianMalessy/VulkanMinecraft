#include "app.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <stdexcept>
#include <array>
#include <math.h>
#include <iostream>
namespace vmc {
	struct simplePushConstantData {
		glm::vec2 offset;
		glm::vec3 color;
	};

	App* app;
	App::App() {
		loadModels();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
		app = this;
	}

	App::~App() {
		app = nullptr;
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

	// draw frame while glfwPollEvents has paused so that we keep drawing as we resize the window
	void App::windowRefreshCallback(GLFWwindow* window) {
		app->drawFrame();
	}

	inline float height(float sidelength) {
		return static_cast<float>(sidelength * sqrt(3) / 2);
	}
	constexpr float sidelength = 0.5f;
	constexpr float midpoint = 0.0f;
	void App::loadModels() {
		glm::vec2 top = { midpoint, -height(sidelength) };
		glm::vec2 left = { midpoint - sidelength, height(sidelength) };
		glm::vec2 right = { midpoint + sidelength, height(sidelength) };
		std::vector<VmcModel::Vertex> vertices = { {top}, {left}, {right} };

		vmcModel = std::make_unique<VmcModel>(vmcDevice, vertices);
	}

	void App::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(simplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(vmcDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void App::createPipeline() {
		assert(vmcSwapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

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

		// wait until the current swapchain isnt being used until we create the new one or also scavenge the old one for resources
		vkDeviceWaitIdle(vmcDevice.device());

		if (vmcSwapChain == nullptr) {
			vmcSwapChain = std::make_unique<VmcSwapChain>(vmcDevice, extent);
		}
		else {
			vmcSwapChain = std::make_unique<VmcSwapChain>(vmcDevice, extent, std::move(vmcSwapChain));
			if (vmcSwapChain->imageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}

		// if render passes are compatible then we don't need to recreate the pipeline
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

	void App::freeCommandBuffers() {
		vkFreeCommandBuffers(vmcDevice.device(), vmcDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	void App::recordCommandBuffer(int imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}

		// A blueprint that tells the graphics pipeline what layout to expect for an output framebuffer + other instructions
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
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		// draw 3 vertices and 1 instance. An instance can be reused to draw copies with the same vertex data
		vmcPipeline->bind(commandBuffers[imageIndex]);
		vmcModel->bind(commandBuffers[imageIndex]);
		for (int i = 0; i < 4; i++) {
			simplePushConstantData push{};
			push.offset = { 0.0f, -0.4f + i * 0.25f };
			push.color = { 0.0f, 0.0f, 0.2f + i * 0.2f };

			vkCmdPushConstants(commandBuffers[imageIndex], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(simplePushConstantData), &push);
			vmcModel->draw(commandBuffers[imageIndex]);
		}


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