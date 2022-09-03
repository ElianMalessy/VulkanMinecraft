#pragma once

#include "vmc_model.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <cstdint>
#include <bitset>
#include <memory>

namespace vmc {
	constexpr unsigned int MAX_ENTITIES = 5000;
	constexpr unsigned int MAX_COMPONENTS = 32;

	using Entity = std::uint32_t;
	using ComponentType = std::uint8_t;
	using Signature = std::bitset<MAX_COMPONENTS>;


	struct BasicObject
	{
		BasicObject() = default;
		BasicObject(BasicObject&&) = default;
		BasicObject& operator=(BasicObject&&) = default;
		glm::vec2 velocity{ 0.0f, 0.0f };
		glm::vec2 acceleration{ 0.0f, 0.0f };
		glm::vec3 color{ 0.0f, 0.0f, 0.0f };
		float mass{ 1.0f };
		std::unique_ptr<VmcModel> model;
	};

	struct Circle : BasicObject {
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
		}
	};

	struct Rect : BasicObject {

		Rect(VmcDevice& device, float l, float w) {
			std::vector<VmcModel::Vertex> vertices{
				{{-w, l}},
				{{-w, -l}},
				{{w, l}},
				{{w, l}},
				{{-w, -l}},
				{{w, -l}},
			};
			this->model = std::make_unique<VmcModel>(device, std::move(vertices));
		}
	};

	struct Gravity
	{
		glm::vec2 force;
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