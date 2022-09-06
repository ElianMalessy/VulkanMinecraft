#pragma once

#include "vmc_model.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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
		Circle(VmcDevice& device, size_t size, float r) {
			std::vector<VmcModel::Vertex> uniqueVertices(size + 1);
			for (int i = 0; i < size; i++) {
				float angle = i * glm::two_pi<float>() / size;
				uniqueVertices[i].position = { r * glm::cos(angle), r * glm::sin(angle) };
			}
			uniqueVertices[size].position = { 0, 0 };

			std::vector<VmcModel::Vertex> vertices;
			for (int i = 0; i < size; i++) {
				vertices.push_back(uniqueVertices[i]);
				vertices.push_back(uniqueVertices[(i + 1) % size]);
				vertices.push_back(uniqueVertices[size]);
			}
			this->model = std::make_unique<VmcModel>(device, std::move(vertices));
			this->radius = r;
		}


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
		glm::vec2 translation{};  // (position offset)
		glm::vec2 scale{ 1.f, 1.f };
		float rotation;

		glm::mat2 mat2() {
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			glm::mat2 rotMatrix{ {c, s}, {-s, c} };

			glm::mat2 scaleMat{ {scale.x, .0f}, {.0f, scale.y} };
			return rotMatrix * scaleMat;
		}
	};

}