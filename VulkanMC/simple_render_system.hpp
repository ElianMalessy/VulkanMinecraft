#pragma once

#include "vmc_pipeline.hpp"
#include "vmc_device.hpp"
#include "vmc_model.hpp"
#include "coordinator.hpp"
#include "types.hpp"
// std
#include <memory>
#include <vector>


namespace vmc {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(VmcDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderEntities(VkCommandBuffer commandBuffer, std::vector<Entity>& entities, Coordinator& coordinator);
	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		VmcDevice& vmcDevice;

		std::unique_ptr<VmcPipeline> vmcPipeline;
		VkPipelineLayout pipelineLayout;
	};
}