#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vmc {
	class VmcCamera {
	public:
		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setPerspectiveProjection(float FOVy, float aspectRatio, float near, float far);

		const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }
	private:
		glm::mat4 projectionMatrix{ 1.f };
	};
}