#pragma once

#include "vmc_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace vmc {
	class VmcModel {
		// takes vertex data from the cpu and allocates memory and copies it over to the gpu
	public:
		struct Vertex {
			glm::vec2 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		VmcModel(VmcDevice& device, const std::vector<Vertex>& vertices);
		~VmcModel();

		VmcModel(const VmcModel&) = delete;
		VmcModel& operator=(const VmcModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);

		VmcDevice& vmcDevice;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;
	};
}