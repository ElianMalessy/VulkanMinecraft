#pragma once

#include "system.hpp"
#include "coordinator.hpp"

namespace vmc {
	void System::Update(float dt, Coordinator& gCoordinator)
	{
		for (auto const& entity : mEntities)
		{
			auto& circle = gCoordinator.GetComponent<Circle>(entity);
			auto& transform = gCoordinator.GetComponent<Transform>(entity);
			auto const& gravity = gCoordinator.GetComponent<Gravity>(entity);

			transform.translation += circle.velocity * dt;

			circle.velocity += gravity.force * dt;
		}
		/*
void circleCollision(Circle circle) {
	float cmbRad = this->radius + circle.radius;
	if (glm::length(this->center - circle.center) > cmbRad) {
		return;
	}
	elasticCollisionVelocity(*this, circle);
}
template<typename A, typename B>
void elasticCollisionVelocity(A& obj1, B& obj2) {
	obj1.velocity = (obj1.velocity * (obj1.mass - obj2.mass) / (obj1.mass + obj2.mass)) + (obj2.velocity * (2 * obj2.mass) / (obj1.mass + obj2.mass));
	obj2.velocity = (obj2.velocity * (obj1.mass - obj2.mass) / (obj1.mass + obj2.mass)) + (obj1.velocity * (2 * obj2.mass) / (obj1.mass + obj2.mass));
}
*/
	};
}