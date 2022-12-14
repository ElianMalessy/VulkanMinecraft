#pragma once

#include "simple_render_system.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
// std
#include <stdexcept>
#include <array>
#include <math.h>
#include <iostream>
namespace vmc {
	// offset for fragment vs vertex data
	// a vec3/vec4 must be aligned to a multiple of 16 bytes, vec2 aligned to a multiple of 8 bytes
	// struct { r, g, b, x, y} is how it is normally, alignas(16) makes it struct {r, g, b, _, x, y}


	SimpleRenderSystem* app;
	SimpleRenderSystem::SimpleRenderSystem(VmcDevice& device, VkRenderPass renderPass) : vmcDevice{ device } {
		createPipelineLayout();
		createPipeline(renderPass);
		app = this;
	}

	SimpleRenderSystem::~SimpleRenderSystem() {
		app = nullptr;
		vkDestroyPipelineLayout(vmcDevice.device(), pipelineLayout, nullptr);
	}

	//void SimpleRenderSystem::windowRefreshCallback(GLFWwindow* window) {
		//app->render();
	//}

	void SimpleRenderSystem::createPipelineLayout() {
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

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		VmcPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		vmcPipeline = std::make_unique<VmcPipeline>(vmcDevice, "default.vert.spv", "default.frag.spv", pipelineConfig);
	}


}
