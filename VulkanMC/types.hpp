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
		float scale;

		// quaternion
		float deg = 0.f;
		glm::vec4 getQuaternion(float offset)
		{
			deg += offset;
			// q = [cos(Q/2), sin(Q/2)v] (where Q is an angle and v is an axis)
			// in this case its y, x, z, w (don't know why y and x are swapped)
			return glm::vec4{ glm::sin(deg) * sqrt(2.f) / 2.f, glm::sin(deg) * sqrt(2.f) / 2.f, 0.f, glm::cos(deg) };
		}
	};

}