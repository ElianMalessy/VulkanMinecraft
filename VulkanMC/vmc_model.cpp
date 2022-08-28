#include "vmc_model.hpp"

#include <cassert>
#include <cstring>
namespace vmc {
	VmcModel::VmcModel(VmcDevice& device, const std::vector<Vertex>& vertices) : vmcDevice{ device } {
		createVertexBuffers(vertices);
	}
	VmcModel::~VmcModel() {
		vkDestroyBuffer(vmcDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(vmcDevice.device(), vertexBufferMemory, nullptr);
	}
	void VmcModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		vmcDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);
		
		// Host = CPU, Device = GPU
		// Host_Visible_Bit makes gpu memory accessible to the Host (CPU)
		// Host_Coherent_Bit updates the gpu memory with our cpu memory which we allocate right now with void* data
		void* data;
		vkMapMemory(vmcDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(vmcDevice.device(), vertexBufferMemory);
	}
	void VmcModel::draw(VkCommandBuffer commandBuffer) {
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}
	void VmcModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}
	std::vector<VkVertexInputBindingDescription> VmcModel::Vertex::getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}
	std::vector<VkVertexInputAttributeDescription> VmcModel::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		// binding is which buffer you use, location is the location of the specific data in the buffer ( layout(location=0) )
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		// color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}
}