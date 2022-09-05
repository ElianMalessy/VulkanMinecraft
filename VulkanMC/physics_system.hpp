#pragma once
#include <entt/entt.hpp>
namespace vmc {

	class PhysicsSystem {
	public:
		template<typename A, typename B>
		inline glm::vec2 calculateGravity(A& obj, B& obj2, glm::vec2 distance) {
			// if we got the dot product of the two different opposite force vectors, it would be zero
			// hence the need for the weird distance code to get r^2 in Fg = Gm1m2 / r^2
			auto distanceSquared = glm::dot(distance, distance);
			return (obj.mass * obj2.mass * 0.001f * -distance) / (glm::sqrt(distanceSquared) * distanceSquared);
		}
		template<typename T>
		inline void elasticCollisionVelocity(entt::entity entity1, entt::entity entity2, entt::registry& registry) {
			auto& obj1 = registry.get<T>(entity1);
			auto& obj2 = registry.get<T>(entity2);

			auto temp = std::move(obj1.velocity);
			obj1.velocity = (obj1.velocity * (obj1.mass - obj2.mass) / (obj1.mass + obj2.mass)) + (obj2.velocity * (2 * obj2.mass) / (obj1.mass + obj2.mass));
			obj2.velocity = (obj2.velocity * (obj1.mass - obj2.mass) / (obj1.mass + obj2.mass)) + (temp * (2 * obj2.mass) / (obj1.mass + obj2.mass));
		}
		template<typename T>
		inline void wallCollisionVelocity(entt::entity entity, entt::registry& registry, glm::vec2 normal) {
			auto& obj = registry.get<T>(entity);
			obj.velocity -= 2.0f * normal * glm::dot(normal, obj.velocity);
		}
		template<typename... Args>
		void Update(float dt, entt::registry& registry)
		{
			([&]
				{
					auto views = registry.view<Args, Transform, Gravity>();
					if (typeid(Args) == typeid(Circle)) {
						for (auto const& entity : views)
						{
							auto& obj = registry.get<Circle>(entity);
							auto& transform = registry.get<Transform>(entity);
							auto& gravity = registry.get<Gravity>(entity);
							for (auto const& e : views) {
								if (e == entity) continue;

								auto& obj2 = registry.get<Circle>(e);
								auto& t = registry.get<Transform>(e);

								auto distance = transform.translation - t.translation;
								gravity.force = calculateGravity(obj, obj2, distance);

								float cmbRad = obj.radius + obj2.radius;
								if (glm::length(distance) <= cmbRad)
									elasticCollisionVelocity<Circle>(entity, e, registry);

								obj.acceleration = gravity.force / obj.mass;
								obj.velocity += obj.acceleration * dt;
								transform.translation += obj.velocity * dt;
								transform.rotation = atan2(obj.acceleration.y, obj.acceleration.x);
							}

							if (glm::abs(transform.translation.x) + obj.radius >= 1.0f) {
								wallCollisionVelocity<Circle>(entity, registry, glm::vec2{ transform.translation.x < 0 ? 1.0f : -1.0f, 0.0f });
							}
							if (glm::abs(transform.translation.y) + obj.radius >= 1.0f) {
								wallCollisionVelocity<Circle>(entity, registry, glm::vec2{ 0.0f, transform.translation.y < 0 ? 1.0f : -1.0f });
							}

						}
					}
					if (typeid(Args) == typeid(Rect)) {
						for (auto const& entity : views) {
							auto& obj = registry.get<Args>(entity);
							if (obj.mass != 1.0f) continue;
							auto& transform = registry.get<Transform>(entity);

							Gravity totalGrav{};
							auto circleView = registry.view<Circle, Transform, Gravity>();
							for (auto const& e : circleView) {
								auto& circle = registry.get<Circle>(e);
								auto& circleTranslation = registry.get<Transform>(e);
								totalGrav.force += calculateGravity(circle, obj, circleTranslation.translation - transform.translation);
							}
							transform.scale.x =
								0.01f + 0.05f * glm::clamp(glm::log(glm::length(totalGrav.force * 500.0f) + 1) / 3.f, 0.f, 1.f);
							transform.rotation = atan2(totalGrav.force.y, totalGrav.force.x);
						}
					}
				} (), ...);
		}

	};
}