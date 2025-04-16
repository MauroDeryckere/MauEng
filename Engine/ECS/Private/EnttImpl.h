#ifndef MAUENG_ENTTIMPL_H
#define MAUENG_ENTTIMPL_H

#include <entt/entt.hpp>
#include "EntityID.h"

namespace MauEng
{
namespace ECS
{
	struct ECSImpl final
	{
		entt::registry registry{};

		[[nodiscard]] EntityID CreateEntity() noexcept
		{
			return static_cast<EntityID>(registry.create());
		}

		void DestroyEntity(EntityID id) noexcept
		{
			registry.destroy(static_cast<entt::entity>(id));
		}

		template <typename ComponentType, typename ... Args>
		ComponentType& AddComponent(EntityID id, Args&&... args) noexcept
		{
			return registry.emplace<ComponentType>(static_cast<entt::entity>(id), std::forward<Args>(args)...);
		}

		template <typename ComponentType>
		[[nodiscard]] ComponentType& GetComponent(EntityID id) const noexcept
		{
			return registry.get<ComponentType>(static_cast<entt::entity>(id));
		}

		template <typename ComponentType>
		void RemoveComponent(EntityID id)
		{
			registry.remove<ComponentType>(static_cast<entt::entity>(id));
		}

		template<typename ComponentType>
		[[nodiscard]] bool HasComponent(EntityID id) const noexcept
		{
			return registry.any_of<ComponentType>(static_cast<entt::entity>(id));
		}
	};
}
}

#endif