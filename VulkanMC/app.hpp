#pragma once

#include "vmc_device.hpp"
#include "vmc_game_object.hpp"
#include "vmc_renderer.hpp"
#include "vmc_window.hpp"
#include "physics_system.hpp"


// std
#include <memory>
#include <vector>

namespace vmc {
	class App {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 800;

		App();
		~App();

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void run();

	private:
		void loadGameObjects();

		VmcWindow vmcWindow{ WIDTH, HEIGHT, "Vulkan Tutorial" };
		VmcDevice vmcDevice{ vmcWindow };
		VmcRenderer vmcRenderer{ vmcWindow, vmcDevice };

		entt::registry registry;
		std::unique_ptr<PhysicsSystem> physicsSystem;
	};
}  // namespace vmc