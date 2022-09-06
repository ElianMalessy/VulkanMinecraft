#pragma once
#include <entt/entt.hpp>
#include "types.hpp"
#include <iostream>
namespace vmc {

	class PhysicsSystem {
	public:

		inline void elasticCollisionVelocity(glm::vec2& v1, glm::vec2& v2, float m1, float m2) {
			auto temp = std::move(v1);
			v1 = (v1 * (m1 - m2) / (m1 + m2)) + (v2 * (2 * m2) / (m1 + m2));
			v2 = (v2 * (m1 - m2) / (m1 + m2)) + (temp * (2 * m2) / (m1 + m2));
		}
		inline void wallCollisionVelocity(glm::vec2& v, glm::vec2 normal) {
			v -= 2.0f * normal * glm::dot(normal, v);
		}

		template<typename... Args>
		void update(float dt, entt::registry& registry, unsigned int substeps = 1)
		{
			const float stepDelta = dt / substeps;
			([&]
				{
					for (unsigned int i = 0; i < substeps; i++)
					{
						stepSimulation<Args>(stepDelta, registry);
					}
				} (), ...);
		}

		template <typename T>
		void vfUpdate(entt::registry& registry) {
			auto vfobj = registry.view<Rect, Transform, Gravity>();
			auto physicsObjs = registry.view<T, Transform, Gravity>();

			for (auto& vf : vfobj) {
				glm::vec2 direction{};
				auto& transformVf = registry.get<Transform>(vf);

				for (auto& obj : physicsObjs) {
					auto [obj, transformObj] = registry.get<T, Transform>(obj);

					direction += calculateGravity(transformObj.translation - transformVf.translation, obj.mass, 1.0f);
				}

				// This scales the length of the field line based on the log of the length
				// values were chosen just through trial and error based on what i liked the look
				// of and then the field line is rotated to point in the direction of the field
				transformVf.scale.x =
					0.005f + 0.045f * glm::clamp(glm::log(glm::length(direction) + 1) / 3.f, 0.f, 1.f);
				transformVf.rotation = atan2(direction.y, direction.x);
			}
		}

		inline glm::vec2 calculateGravity(glm::vec2 distance, float m1, float m2) const
		{
			float distanceSquared = glm::dot(distance, distance);
			if (glm::abs(distanceSquared) < 1e-3f) return { .0f, .0f };
			return (0.4f * m1 * m2 / distanceSquared) * distance / glm::sqrt(distanceSquared);
		}

	private:

		template<typename T>
		void stepSimulation(float dt, entt::registry& registry)
		{
			bool hasCollided = false;
			auto view = registry.view<T, Transform, Gravity>();
			for (auto& entity1 : view)
			{
				auto [obj1, transform1, gravity1] = registry.get<T, Transform, Gravity>(entity1);

				for (auto& entity2 : view)
				{
					if (entity1 == entity2)
						continue;

					auto [obj2, transform2, gravity2] = registry.get<T, Transform, Gravity>(entity2);

					auto distance = transform1.translation - transform2.translation;
					auto force = calculateGravity(distance, obj1.mass, obj2.mass);
					obj1.velocity += dt * -force / obj1.mass;
					obj2.velocity += dt * force / obj2.mass;
					if (typeid(T).name() == typeid(Circle).name() && glm::length(distance) <= glm::abs(obj1.radius + obj2.radius) && hasCollided == false) {
						elasticCollisionVelocity(obj1.velocity, obj2.velocity, obj1.mass, obj2.mass);
						hasCollided = true;
					}

				}
				if (typeid(T).name() == typeid(Circle).name()) {
					if (glm::abs(transform1.translation.x) + obj1.radius >= 1.0f) {
						wallCollisionVelocity(obj1.velocity, glm::vec2{ transform1.translation.x < 0 ? 1.0f : -1.0f, 0.0f });
					}
					if (glm::abs(transform1.translation.y) + obj1.radius >= 1.0f) {
						wallCollisionVelocity(obj1.velocity, glm::vec2{ 0.0f, transform1.translation.y < 0 ? 1.0f : -1.0f });
					}
				}

			}
			// update each objects position based on its final velocity	
			for (auto& entity : view)
			{
				auto [obj, transform] = registry.get<T, Transform>(entity);
				transform.translation += dt * obj.velocity;
			}

		}
	};
}
