#pragma once

#include "system.hpp"
#include "coordinator.hpp"
#include <iostream>
namespace vmc {
	class PhysicsSystem : public System {
	public:

		void Update(float dt, Coordinator& coordinator)
		{
			for (auto const& entity : mEntities)
			{
				auto& circle = coordinator.GetComponent<Circle>(entity);
				auto& transform = coordinator.GetComponent<Transform>(entity);
				auto const& gravity = coordinator.GetComponent<Gravity>(entity);

				transform.translation += circle.velocity * dt;
				circle.velocity += gravity.force * dt;
			}
		};
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