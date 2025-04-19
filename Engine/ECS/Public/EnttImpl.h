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

#pragma region Registry
		template<typename... ComponentTypes>
		void Compact() noexcept
		{
			registry.compact<ComponentTypes...>();
		}

		template<typename ComponentType>
		[[nodiscard]] std::size_t ComponentCount() const noexcept
		{
			return registry.view<ComponentType>().size();
		}

		template <typename... ComponentTypes>
		void Clear() noexcept
		{
			registry.clear<ComponentTypes...>();
		}

		template<typename ComponentType, typename It>
		void Insert(It first, It last, ComponentType const& component)
		{
			registry.insert<ComponentType>(first, last, component);
		}

		template<typename... ComponentTypes>
		[[nodiscard]] bool IsOwned() const noexcept
		{
			return registry.owned<ComponentTypes...>();
		}	

#pragma endregion
		
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

		template <typename... ComponentTypes>
		[[nodiscard]] bool HasAllOfComponents(EntityID id) const noexcept
		{
			return registry.all_of<ComponentTypes...>(static_cast<entt::entity>(id));
		}
		template <typename... ComponentTypes>
		[[nodiscard]] bool HasAnyOfComponents(EntityID id) const noexcept
		{
			return registry.any_of<ComponentTypes...>(static_cast<entt::entity>(id));
		}

		template<typename FirstComponentType, typename... OtherComponentTypes>
		void Erase(EntityID id) noexcept
		{
			registry.erase<FirstComponentType, OtherComponentTypes...>(static_cast<entt::entity>(id));
		}

		template<typename FirstComponentType, typename... OtherComponentTypes, typename Iterator>
		void Erase(Iterator begin, Iterator end) noexcept
		{
			registry.erase<FirstComponentType, OtherComponentTypes...>(begin, end);
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

		template <typename... ComponentTypes>
		[[nodiscard]] bool RemoveComponent(EntityID id) noexcept
		{
			return registry.remove<ComponentTypes...>(static_cast<entt::entity>(id)) == sizeof...(ComponentTypes);
		}
		template <typename... ComponentTypes, typename Iterator>
		[[nodiscard]] bool RemoveComponent(Iterator begin, Iterator end) noexcept
		{
			return registry.remove<ComponentTypes...>(begin, end) == sizeof...(ComponentTypes);
		}

		template<typename ComponentType>
		[[nodiscard]] bool HasComponent(EntityID id) const noexcept
		{
			return registry.any_of<ComponentType>(static_cast<entt::entity>(id));
		}

		template<typename ComponentType>
		[[nodiscard]] ComponentType const* TryGetComponent(EntityID id) const noexcept
		{
			return registry.try_get<ComponentType>(static_cast<entt::entity>(id));
		}
		template<typename ComponentType>
		[[nodiscard]] ComponentType* TryGetComponent(EntityID id) noexcept
		{
			return registry.try_get<ComponentType>(static_cast<entt::entity>(id));
		}

		template<typename ComponentType, typename... Args>
		[[nodiscard]] ComponentType& ReplaceComponent(EntityID id, Args&&... args) noexcept
		{
			return registry.replace<ComponentType>(static_cast<entt::entity>(id), std::forward<Args>(args)...);
		}

		template<typename ComponentType, typename... Args>
		[[nodiscard]] ComponentType& AddOrReplaceComponent(EntityID id, Args&&... args) noexcept
		{
			return registry.emplace_or_replace<ComponentType>(static_cast<entt::entity>(id), std::forward<Args>(args)...);
		}

		template<typename ComponentType, typename... Args>
		[[nodiscard]] ComponentType& GetOrEmplaceComponent(EntityID id, Args&&... args) noexcept
		{
			return registry.get_or_emplace<ComponentType>(static_cast<entt::entity>(id), std::forward<Args>(args)...);
		}

		template<typename ComponentType1, typename ComponentType2>
		void Sort() noexcept
		{
			registry.sort<ComponentType1, ComponentType2>();
		}
		template<typename ComponentType, typename Comparator>
		void Sort(Comparator comp) noexcept
		{
			registry.sort<ComponentType>(comp);
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
