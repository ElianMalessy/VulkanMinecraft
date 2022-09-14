#pragma once

#include "vmc_model.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <cstdint>
#include <memory>
#include <iostream>

namespace vmc {
	class BasicObject
	{
	public:
		BasicObject() = default;
		BasicObject(BasicObject&&) = default;
		BasicObject& operator=(BasicObject&&) = default;
		BasicObject(glm::vec2 v, glm::vec2 a, glm::vec3 c, float m) : velocity{ v }, acceleration{ a }, color{ c }, mass{ m } {}
		glm::vec2 velocity{ 0.0f, 0.0f };
		glm::vec2 acceleration{ 0.0f, 0.0f };
		glm::vec3 color{ 0.0f, 0.0f, 0.0f };
		float mass{ 1.0f };
		std::shared_ptr<VmcModel> model;

	};

	struct Circle : BasicObject {
		float radius{ 0.5f };
		Circle() = default;

	};
	struct Rect : BasicObject {
		glm::vec3 color = glm::vec3(1.0f);
		Rect() = default;
		/*
		Rect(VmcDevice& device, float l, float w, glm::vec2 offset, float mass) {
			std::vector<VmcModel::Vertex> vertices{
				{{-w, l}},
				{{-w, -l}},
				{{w, l}},
				{{w, l}},
				{{-w, -l}},
				{{w, -l}},
			};
			for (auto& v : vertices) {
				v.position += offset;
			}
			this->model = std::make_unique<VmcModel>(device, std::move(vertices));
			this->mass = mass;
		}
		*/
	};

	struct Gravity
	{
		glm::vec2 force{ 0.0f, 0.0f };
	};

	struct Transform {
		glm::vec3 translation{};  // (position offset)
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};


		// quaternion
		float deg = 1.f;
		glm::mat4 getModelMatrix(float offset)
		{
			deg += offset;
			glm::mat4 t = glm::translate(glm::mat4{ 1.f }, translation);
			glm::mat4 r = glm::toMat4(glm::quat{ glm::cos(deg), 0, glm::sin(deg), 0 });
			glm::mat4 s = glm::scale(glm::mat4{ 1.f }, scale);
			return t * r * s;
		}
	};

}