#pragma once

#include "vmc_pipeline.hpp"
#include "vmc_device.hpp"
#include "vmc_model.hpp"
#include "types.hpp"
#include <entt/entt.hpp>
// std
#include <memory>
#include <vector>


namespace vmc {
	struct simplePushConstantData {
		glm::mat4 transform{ 1.f }; // 2x2 identity matrix
		alignas(16) glm::vec3 color;
	};
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(VmcDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		template<typename... Args>
		void renderEntities(VkCommandBuffer& commandBuffer, entt::registry& registry) {
			vmcPipeline->bind(commandBuffer);
			([&]
				{
					auto views = registry.view<Args, Transform>();
					for (auto& entity : views) {

						auto& obj = views.get<Args>(entity);
						auto& transform = views.get<Transform>(entity);
						//auto const& gravity = views.get<Gravity>(entity);
						transform.rotation.y = glm::mod(transform.rotation.y + 0.01f, glm::two_pi<float>());
						simplePushConstantData push{};
						push.color = obj.color;
						push.transform = transform.getModelMatrix(0.01f);

						vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(simplePushConstantData), &push);
						obj.model->bind(commandBuffer);
						obj.model->draw(commandBuffer);
					}

				} (), ...);
		}
	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		VmcDevice& vmcDevice;

		std::unique_ptr<VmcPipeline> vmcPipeline;
		VkPipelineLayout pipelineLayout;
	};
}
