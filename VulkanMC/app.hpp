#pragma once

#include "vmc_window.hpp"
#include "vmc_pipeline.hpp"
#include "vmc_device.hpp"
#include "vmc_swap_chain.hpp"
#include "vmc_model.hpp"

#include <memory>
#include <vector>
#include <functional>

namespace vmc {
	class App {
	public:
		static constexpr uint32_t HEIGHT = 800;
		static constexpr uint32_t WIDTH = 800;

		App();
		~App();

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void run();
		void drawFrame();
	private:
		static void windowRefreshCallback(GLFWwindow* window);

		VmcWindow vmcWindow{ WIDTH, HEIGHT, "Hello Vulkan", &windowRefreshCallback };
		void loadModels();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void recreateSwapChain();
		void recordCommandBuffer(int imageIndex);

		VmcDevice vmcDevice{ vmcWindow };
		std::unique_ptr<VmcSwapChain> vmcSwapChain;
		std::unique_ptr<VmcPipeline> vmcPipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::unique_ptr<VmcModel> vmcModel;
	};
}
