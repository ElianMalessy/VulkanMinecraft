#include "app.hpp"
#include "simple_render_system.hpp"
#include "physics_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <stdexcept>
#include <array>
#include <math.h>
#include <iostream>
namespace vmc {
	App::App() {
		loadGameObjects();
	}

	App::~App() { }


	void App::run() {
		SimpleRenderSystem simpleRenderSystem{ vmcDevice, vmcRenderer.getSwapChainRenderPass() };



		//float dt = 0.0f;

		while (!vmcWindow.shouldClose()) {
			glfwPollEvents();
			// the beginFrame function returns a nullptr if the swapchain needs to be recreated
			//auto startTime = std::chrono::high_resolution_clock::now();

			//physicsSystem->Update(dt);

			//auto stopTime = std::chrono::high_resolution_clock::now();

			//dt = std::chrono::duration<float, std::chrono::seconds::period>(stopTime - startTime).count();
			if (auto commandbuffer = vmcRenderer.beginFrame()) {
				vmcRenderer.beginSwapChainRenderPass(commandbuffer);
				simpleRenderSystem.renderEntities(commandbuffer, entities, gCoordinator);
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
		gCoordinator.RegisterComponent<Gravity>();
		gCoordinator.RegisterComponent<Circle>();
		gCoordinator.RegisterComponent<Transform>();

		auto physicsSystem = gCoordinator.RegisterSystem<System>();

		Signature signature;
		signature.set(gCoordinator.GetComponentType<Gravity>());
		signature.set(gCoordinator.GetComponentType<Circle>());
		signature.set(gCoordinator.GetComponentType<Transform>());
		gCoordinator.SetSystemSignature<System>(signature);

		auto blueEntity = gCoordinator.CreateEntity();
		auto redEntity = gCoordinator.CreateEntity();

		constexpr float r = 0.2f;
		constexpr size_t size = 64;

		Circle blueCircle(vmcDevice, size, r);
		blueCircle.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		blueCircle.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
		blueCircle.color = { .1f, .1f, .8f };


		Circle redCircle(vmcDevice, size, r);
		redCircle.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		redCircle.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
		redCircle.color = { .8f, .1f, .1f };

		gCoordinator.AddComponent(blueEntity, std::move(blueCircle));
		gCoordinator.AddComponent(blueEntity, Gravity{ glm::vec3(0.0f, 9.8f, 0.0f) });
		gCoordinator.AddComponent(blueEntity, Transform{ glm::vec2(-0.25f, 0.0f) });
		gCoordinator.AddComponent(redEntity, std::move(redCircle));
		gCoordinator.AddComponent(redEntity, Gravity{ glm::vec3(0.0f, 9.8f, 0.0f) });
		gCoordinator.AddComponent(redEntity, Transform{ glm::vec2(0.25f, 0.0f) });

		entities.push_back(blueEntity);
		entities.push_back(redEntity);
	}
}

