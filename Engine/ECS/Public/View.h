#ifndef MAUENGECS_VIEW_H
#define MAUENGECS_VIEW_H

#include "EntityID.h"
#include <memory>
#include <concepts>

#include "Asserts/Asserts.h"

#include "EnttImpl.h"

namespace MauEng::ECS
{
	using InternalEntityType = entt::entity;

	template<typename... ExcludeTypes>
	using ExcludeType = entt::exclude_t<ExcludeTypes...>;

	template<typename... ComponentTypes>
	class ViewWrapper
	{
	public:
		using ViewType = entt::view<entt::get_t<ComponentTypes...>>;


		explicit ViewWrapper(ViewType view)
			: m_View{ view } {
		}

		/**
		 * @brief iterate over all entities with the given components
		 * @tparam Func function type (usually automatically deduced)
		 * @param func function to execute for each entity
		*/
		template<typename Func>
			requires  (std::is_invocable_v<Func, ComponentTypes&...>
					|| std::is_invocable_v<Func, EntityID, ComponentTypes&...>)
		void Each(Func&& func) noexcept
		{
			if constexpr (std::is_invocable_v<Func, EntityID, ComponentTypes&...>)
			{
				m_View.each([&](entt::entity id, ComponentTypes&... comps)
					{
						func(static_cast<EntityID>(id), comps...);
					});
			}
			else if constexpr (std::is_invocable_v<Func, ComponentTypes&...>)
			{
				m_View.each([&](entt::entity, ComponentTypes&... comps)
					{
						func(comps...);
					});
			}
		}

		/**
		 * @brief iterate over all entities with the given components
		 * @tparam Func function type (usually automatically deduced)
		 * @param func function to execute for each entity
		*/
		template<typename Func>
			requires ( std::is_invocable_v<Func, ComponentTypes const&...>
					|| std::is_invocable_v<Func, EntityID, ComponentTypes const&...>)
			void Each(Func&& func) const noexcept
		{
			if constexpr (std::is_invocable_v<Func, EntityID, ComponentTypes const&...>)
			{
				m_View.each([&](entt::entity id, ComponentTypes const&... comps)
					{
						func(static_cast<EntityID>(id), comps...);
					});
			}
			else if constexpr (std::is_invocable_v<Func, ComponentTypes&...>)
			{
				m_View.each([&](entt::entity, ComponentTypes const&... comps)
					{
						func(comps...);
					});
			}
		}

		template<typename ComponentType>
		[[nodiscard]] ComponentType& Get(EntityID id) const noexcept
		{
			return m_View.template get<ComponentType>(static_cast<InternalEntityType>(id));
		}

		[[nodiscard]] auto begin() const noexcept { return m_View.begin(); }
		[[nodiscard]] auto rbegin() const noexcept { return m_View.rbegin(); }
		[[nodiscard]] auto end() const noexcept { return m_View.end(); }
		[[nodiscard]] auto rend() const noexcept { return m_View.rend(); }

	private:
		ViewType m_View;
	};
}

#endif