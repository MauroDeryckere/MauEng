#ifndef MAUENGECS_GROUP_H
#define MAUENGECS_GROUP_H

#include "EntityID.h"
#include <memory>
#include <concepts>

#include "Asserts/Asserts.h"
#include "EnttImpl.h"
#include "View.h"

namespace MauEng::ECS
{
	template<typename... ComponentTypes>
	class GroupWrapper final
	{
	public:
		using GroupType = decltype(std::declval<entt::registry>().group<ComponentTypes...>(entt::get_t{}, entt::exclude_t{}));
		explicit GroupWrapper(GroupType group)
			: m_Group{ group } {
		}

		/**
		 * @brief Iterate over all entities with the given components (const version)
		 * @tparam Func Function type (usually automatically deduced)
		 * @param func Function to execute for each entity
		*/
		template<typename Func>
			requires (std::is_invocable_v<Func, ComponentTypes const&...>
				|| std::is_invocable_v<Func, EntityID, ComponentTypes const&...>)
		void Each(Func&& func) const noexcept
		{
			if constexpr (std::is_invocable_v<Func, EntityID, ComponentTypes const&...>)
			{
				m_Group.each([&](entt::entity id, ComponentTypes const&... comps)
					{
						func(static_cast<EntityID>(id), comps...);
					});
			}
			else if constexpr (std::is_invocable_v<Func, ComponentTypes const&...>)
			{
				m_Group.each([&](entt::entity, ComponentTypes const&... comps)
					{
						func(comps...);
					});
			}
		}

		/**
		 * @brief Iterate over all entities with the given components
		 * @tparam Func Function type (usually automatically deduced)
		 * @param func Function to execute for each entity
		*/
		template<typename Func>
			requires ( std::is_invocable_v<Func, ComponentTypes&...>
					|| std::is_invocable_v<Func, EntityID, ComponentTypes&...>)
		void Each(Func&& func) noexcept
		{
			if constexpr (std::is_invocable_v<Func, EntityID, ComponentTypes&...>)
			{
				m_Group.each([&](entt::entity id, ComponentTypes&... comps)
					{
						func(static_cast<EntityID>(id), comps...);
					});
			}
			else if constexpr (std::is_invocable_v<Func, ComponentTypes&...>)
			{
				m_Group.each([&](entt::entity, ComponentTypes&... comps)
					{
						func(comps...);
					});
			}
		}

		template<typename ComponentType>
		[[nodiscard]] ComponentType& Get(EntityID id) const noexcept
		{
			return m_Group.template get<ComponentType>(static_cast<InternalEntityType>(id));
		}

		[[nodiscard]] auto begin() const noexcept { return m_Group.begin(); }
		[[nodiscard]] auto rbegin() const noexcept { return m_Group.rbegin(); }
		[[nodiscard]] auto end() const noexcept { return m_Group.end(); }
		[[nodiscard]] auto rend() const noexcept { return m_Group.rend(); }

	private:
		GroupType m_Group;
	};
}

#endif