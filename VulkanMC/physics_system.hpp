#pragma once
#include <entt/entt.hpp>
#include "types.hpp"
#include <iostream>
namespace vmc {

	class PhysicsSystem {
	public:

		template<typename T>
		inline void elasticCollisionVelocity(entt::entity entity1, entt::entity entity2, entt::registry& registry) {
			auto& obj1 = registry.get<T>(entity1);
			auto& obj2 = registry.get<T>(entity2);

			obj1.velocity = (obj1.velocity * (obj1.mass - obj2.mass) / (obj1.mass + obj2.mass)) + (obj2.velocity * (2 * obj2.mass) / (obj1.mass + obj2.mass));
			obj2.velocity = (obj2.velocity * (obj1.mass - obj2.mass) / (obj1.mass + obj2.mass)) + (-obj1.velocity * (2 * obj2.mass) / (obj1.mass + obj2.mass));

		}
		template<typename T>
		inline void wallCollisionVelocity(entt::entity entity, entt::registry& registry, glm::vec2 normal) {
			auto& obj = registry.get<T>(entity);
			obj.velocity -= 2.0f * normal * glm::dot(normal, obj.velocity);
		}

		template<typename Args>
		void update(float dt, entt::registry& registry, unsigned int substeps = 1)
		{
			const float stepDelta = dt / substeps;

			for (unsigned int i = 0; i < substeps; i++)
			{
				stepSimulation<Circle>(stepDelta, registry);
			}
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
			for (auto& entityA : view)
			{
				auto [objA, transformA, gravityA] = registry.get<T, Transform, Gravity>(entityA);

				for (auto& entityB : view)
				{
					if (entityA == entityB)
						continue;

					auto [objB, transformB, gravityB] = registry.get<T, Transform, Gravity>(entityB);

					auto distance = transformA.translation - transformB.translation;
					auto force = calculateGravity(distance, objA.mass, objB.mass);
					objA.velocity += dt * -force / objA.mass;
					objB.velocity += dt * force / objB.mass;
					if (typeid(T).name() == typeid(Circle).name() && glm::length(distance) <= glm::abs(objA.radius + objB.radius) && hasCollided == false) {
						elasticCollisionVelocity<Circle>(entityA, entityB, registry);
						hasCollided = true;
					}

				}
				if (typeid(T).name() == typeid(Circle).name()) {
					if (glm::abs(transformA.translation.x) + objA.radius >= 1.0f) {
						wallCollisionVelocity<Circle>(entityA, registry, glm::vec2{ transformA.translation.x < 0 ? 1.0f : -1.0f, 0.0f });
					}
					if (glm::abs(transformA.translation.y) + objA.radius >= 1.0f) {
						wallCollisionVelocity<Circle>(entityA, registry, glm::vec2{ 0.0f, transformA.translation.y < 0 ? 1.0f : -1.0f });
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
