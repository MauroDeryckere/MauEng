#ifndef MAUENG_ENTTIMPL_H
#define MAUENG_ENTTIMPL_H

//TODO fix include
#include "../../ECS/Libs/Entt/single_include/entt/entt.hpp"
#include "EntityID.h"

namespace MauEng::ECS
{
	struct ECSImpl final
	{
		entt::registry registry{};

#pragma region Entities
		[[nodiscard]] EntityID CreateEntity() noexcept
		{
			return static_cast<EntityID>(registry.create());
		}

		void DestroyEntity(EntityID id) noexcept
		{
			registry.destroy(static_cast<entt::entity>(id));
		}

		[[nodiscard]] bool IsValid(EntityID id) const noexcept
		{
			return registry.valid(static_cast<entt::entity>(id));
		}
#pragma endregion

#pragma region Components
		template <typename ComponentType, typename ... Args>
		ComponentType& AddComponent(EntityID id, Args&&... args) noexcept
		{
			return registry.emplace<ComponentType>(static_cast<entt::entity>(id), std::forward<Args>(args)...);
		}

		template <typename ComponentType>
		[[nodiscard]] ComponentType const& GetComponent(EntityID id) const noexcept
		{
			return registry.get<ComponentType>(static_cast<entt::entity>(id));
		}
		template <typename ComponentType>
		[[nodiscard]] ComponentType& GetComponent(EntityID id) noexcept
		{
			return registry.get<ComponentType>(static_cast<entt::entity>(id));
		}

		template <typename ComponentType>
		bool RemoveComponent(EntityID id) noexcept
		{
			return registry.remove<ComponentType>(static_cast<entt::entity>(id)) == 1;
		}

		template<typename ComponentType>
		[[nodiscard]] bool HasComponent(EntityID id) const noexcept
		{
			return registry.any_of<ComponentType>(static_cast<entt::entity>(id));
		}
#pragma endregion

#pragma region ViewsAndGroups
		template<typename... ComponentTypes, typename... ExcludeTypes>
		[[nodiscard]] auto View(entt::exclude_t<ExcludeTypes...> exclude = entt::exclude_t{}) noexcept
		{
			return registry.view<ComponentTypes...>(exclude);
		}

		template<typename... Owned, typename... Get, typename... ExcludeTypes>
		[[nodiscard]] auto Group(
			entt::get_t<Get...> get = entt::get_t{},
			entt::exclude_t<ExcludeTypes...> exclude = entt::exclude_t{}) noexcept
		{
			return registry.group<Owned...>(get, exclude);
		}
#pragma endregion
	};
}

#endif
