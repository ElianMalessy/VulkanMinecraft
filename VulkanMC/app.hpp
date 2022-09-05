#pragma once

#include "vmc_window.hpp"
#include "vmc_device.hpp"
#include "vmc_model.hpp"
#include "vmc_renderer.hpp"
#include "simple_render_system.hpp"
#include "physics_system.hpp"
// std
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
		void render(SimpleRenderSystem& simpleRenderSystem);

	private:
		//static void windowRefreshCallback(GLFWwindow* window);
		VmcWindow vmcWindow{ WIDTH, HEIGHT, "VulkanMC" };

		void loadGameObjects();

		VmcDevice vmcDevice{ vmcWindow };
		VmcRenderer vmcRenderer{ vmcWindow, vmcDevice };
		entt::registry registry;
		std::unique_ptr<PhysicsSystem> physicsSystem;
	};
}
