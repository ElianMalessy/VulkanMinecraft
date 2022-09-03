#pragma once

#include <set>

namespace vmc {
	class Coordinator;
	class System
	{
	public:
		std::set<Entity> mEntities;

		void Update(float dt, Coordinator& gCoordinator);

	};
}
