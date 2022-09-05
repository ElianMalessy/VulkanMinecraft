#include "app.hpp"
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <stdexcept>
#include <chrono>
#include <thread>
#include <mutex>

namespace vmc {
	App::App() {
		loadGameObjects();
	}

	App::~App() { }

	void App::run() {
		SimpleRenderSystem simpleRenderSystem{ vmcDevice, vmcRenderer.getSwapChainRenderPass() };

		float dt = 0.0f;
		auto startTime = std::chrono::steady_clock::now();

		while (!vmcWindow.shouldClose()) {
			glfwPollEvents();

			auto stopTime = std::chrono::steady_clock::now();
			dt = std::chrono::duration_cast<std::chrono::duration<float>>(stopTime - startTime).count();
			startTime = std::chrono::steady_clock::now();
			physicsSystem->Update<Circle, Rect>(dt, registry);

			// the beginFrame function returns a nullptr if the swapchain needs to be recreated
			if (auto commandbuffer = vmcRenderer.beginFrame()) {
				vmcRenderer.beginSwapChainRenderPass(commandbuffer);
				simpleRenderSystem.renderEntities<Circle, Rect>(commandbuffer, registry);
				vmcRenderer.endSwapChainRenderPass(commandbuffer);
				vmcRenderer.endFrame();
			}
		}
		// cpu will wait until all gpu operations have been completed
		vkDeviceWaitIdle(vmcDevice.device());
	}

	// draw frame while glfwPollEvents has paused so that we keep drawing as we resize the window
	//void App::windowRefreshCallback(GLFWwindow* window) {
		//app->render();
	//}


	void App::loadGameObjects() {
		constexpr float r = 0.2f;
		constexpr size_t size = 64;

		Circle blueCircle = Circle(vmcDevice, size, r);
		blueCircle.velocity.x = 0.25f;
		blueCircle.color = { .1f, .1f, .9f };

		Circle redCircle = Circle(vmcDevice, size, r);
		redCircle.velocity = { -0.1f, 0.1f };
		redCircle.color = { .8f, .1f, .1f };

		auto blueEntity = registry.create();
		auto redEntity = registry.create();

		registry.emplace<Circle>(blueEntity, std::move(blueCircle));
		registry.emplace<Gravity>(blueEntity);
		registry.emplace<Transform>(blueEntity, glm::vec2(-0.5f, 0.0f));

		registry.emplace<Circle>(redEntity, std::move(redCircle));
		registry.emplace<Gravity>(redEntity);
		registry.emplace<Transform>(redEntity, glm::vec2(0.5f, 0.25f));

		registry.emplace<Rect>(blueEntity, Rect(vmcDevice, 0.01f, 0.15f, { .25f, .0f }, 0.0f));
		//registry.emplace<Transform>(blueEntity, glm::vec2(-0.5f, 0.0f));

		registry.emplace<Rect>(redEntity, Rect(vmcDevice, 0.01f, 0.15f, { .25f, .0f }, 0.0f));
		//registry.emplace<Transform>(redEntity, glm::vec2(-0.5f, 0.25f));

		int gridCount = 40;
		for (int i = 0; i < gridCount; i++) {
			for (int j = 0; j < gridCount; j++) {
				auto vfEntity = registry.create();
				Rect vf = Rect(vmcDevice, 0.5f, 0.5f, { .5f, .0f }, 1.0f);
				vf.color = { 1.0f, 1.0f, 1.0f };

				registry.emplace<Rect>(vfEntity, std::move(vf));
				registry.emplace<Transform>(vfEntity, glm::vec2(-1.0f + (i + 0.5f) * 2.0f / gridCount,
					-1.0f + (j + 0.5f) * 2.0f / gridCount), glm::vec2(0.008f));
				registry.emplace<Gravity>(vfEntity);
			}
		}

	}
}