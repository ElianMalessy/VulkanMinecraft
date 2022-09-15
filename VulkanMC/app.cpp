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

			//physicsSystem->update<Circle>(1.f / 60, registry, 5);
			//physicsSystem->vfUpdate<Circle>(registry);

			// the beginFrame function returns a nullptr if the swapchain needs to be recreated
			if (auto commandbuffer = vmcRenderer.beginFrame()) {
				vmcRenderer.beginSwapChainRenderPass(commandbuffer);
				simpleRenderSystem.renderEntities<Rect>(commandbuffer, registry);
				vmcRenderer.endSwapChainRenderPass(commandbuffer);
				vmcRenderer.endFrame();
			}
			//auto et = std::chrono::steady_clock::now();
			//std::cout << 1 / std::chrono::duration_cast<std::chrono::duration<float>>(et - stopTime).count() << " ";

		}
		// cpu will wait until all gpu operations have been completed
		vkDeviceWaitIdle(vmcDevice.device());
	}

	// draw frame while glfwPollEvents has paused so that we keep drawing as we resize the window
	//void App::windowRefreshCallback(GLFWwindow* window) {
		//app->render();
	//}

	std::unique_ptr<VmcModel> createCubeModel(VmcDevice& device, glm::vec3 offset) {
		std::vector<VmcModel::Vertex> vertices{

			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

		};
		for (auto& v : vertices) {
			v.position += offset;
		}
		return std::make_unique<VmcModel>(device, vertices);
	}
	void App::loadGameObjects() {
		std::unique_ptr<VmcModel> cubeModel = createCubeModel(vmcDevice, { .0f, .0f, .0f });
		auto cubeEntity = registry.create();
		Rect r;
		r.model = std::move(cubeModel);
		registry.emplace<Rect>(cubeEntity, std::move(r));
		registry.emplace<Transform>(cubeEntity, Transform{ {.0f, .0f, 0.5f, .25f } });
	}
}