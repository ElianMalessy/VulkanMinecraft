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
		glm::vec4 quaternion{ 1.f };
		alignas(16) glm::vec3 color;
		alignas(16) glm::vec3 translate;
		float scale;
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
						simplePushConstantData push{};
						push.color = obj.color;
						push.quaternion = transform.getQuaternion(0.01f);
						push.translate = transform.translation;
						push.scale = transform.scale;


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
