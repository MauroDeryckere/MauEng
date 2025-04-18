#ifndef MAUENGECS_VIEW_H
#define MAUENGECS_VIEW_H

#include "EntityID.h"
#include <memory>
#include <concepts>
#include <execution>

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


		explicit ViewWrapper(ViewType& view)
			: m_View{ view } {
			ME_ASSERT(m_View);
		}

		/**
		 * @brief Iterate over all entities with the given components
		 * @tparam Func Function type (usually automatically deduced)
		 * @tparam ExecPolicy Execution policy when looping over elements multithreaded (usually automatically deduced)
		 * @param func Function to execute for each entity
		 * @param policy Policy to multithread with
		*/
		template<typename Func, typename ExecPolicy = std::execution::sequenced_policy>
			requires (std::is_invocable_v<Func, ComponentTypes&...>
		|| std::is_invocable_v<Func, EntityID, ComponentTypes&...>
			|| std::is_invocable_v <Func>)
			&& std::is_execution_policy_v<std::remove_cvref_t<ExecPolicy>>
			void Each(Func&& func, ExecPolicy policy = ExecPolicy{}) const noexcept
		{
			// If we are caling the functon unsequential, use std::foreach
			if constexpr (!std::is_same_v<ExecPolicy, std::execution::sequenced_policy>)
			{
				auto const parallelFuncCall{ [&](InternalEntityType entity)
					{
						if constexpr(sizeof...(ComponentTypes) > 1)
						{
							static_assert(std::is_same_v<
								decltype(m_View.template get<ComponentTypes...>(InternalEntityType{})),
								std::tuple<ComponentTypes&...>
							> , "View::get<ComponentTypes...> must return a tuple of references");

							std::apply(
								[&](ComponentTypes&... comps)
								{
									if constexpr (std::is_invocable_v<Func, EntityID, ComponentTypes&...>)
									{
										func(static_cast<EntityID>(entity), comps...);
									}
									else if constexpr (std::is_invocable_v<Func, ComponentTypes&...>)
									{
										func(comps...);
									}
									else if constexpr (std::is_invocable_v<Func>)
									{
										func();
									}
								},
								m_View.template get<ComponentTypes...>(entity)
							);
						}
						else
						{
							if constexpr (std::is_invocable_v<Func, EntityID, ComponentTypes&...>)
							{
								func(static_cast<EntityID>(entity), m_View.template get<ComponentTypes...>(entity));
							}
							else if constexpr (std::is_invocable_v<Func, ComponentTypes&...>)
							{
								func(m_View.template get<ComponentTypes...>(entity));
							}
							else if constexpr (std::is_invocable_v<Func>)
							{
								func();
							}
						}

					} };

				std::for_each(policy, m_View.begin(), m_View.end(), parallelFuncCall);
			}
			else
			{
				if constexpr (std::is_invocable_v<Func, EntityID, ComponentTypes&...>)
				{
					m_View.each([&](InternalEntityType id, ComponentTypes&... comps)
						{
							func(static_cast<EntityID>(id), comps...);
						});
				}
				else if constexpr (std::is_invocable_v<Func, ComponentTypes&...>)
				{
					m_View.each([&](ComponentTypes&... comps)
						{
							func(comps...);
						});
				}
				else if constexpr (std::is_invocable_v<Func>)
				{
					m_View.each([&](ComponentTypes&... comps)
						{
							func();
						});
				}
			}
		}

		template<typename ComponentType>
		[[nodiscard]] ComponentType& Get(EntityID id) const noexcept
		{
			return m_View.template get<ComponentType>(static_cast<InternalEntityType>(id));
		}
		template<typename ComponentType>
		[[nodiscard]] ComponentType* TryGet(EntityID id) const noexcept
		{
			return m_View.template try_get<ComponentType>(static_cast<InternalEntityType>(id));
		}

		[[nodiscard]] bool Contains(EntityID id) const noexcept
		{
			return m_View.contains(static_cast<InternalEntityType>(id));
		}

		template<typename Func>
		void Where(Func&& func) noexcept
		{
			m_View.where(std::forward<Func>(func));
		}

		[[nodiscard]] bool Empty() const noexcept { return m_View.empty(); }
		[[nodiscard]] std::size_t Size() const noexcept { return m_View.size(); }

		[[nodiscard]] auto begin() const noexcept { return m_View.begin(); }
		[[nodiscard]] auto rbegin() const noexcept { return m_View.rbegin(); }
		[[nodiscard]] auto end() const noexcept { return m_View.end(); }
		[[nodiscard]] auto rend() const noexcept { return m_View.rend(); }

	private:
		ViewType m_View;
	};
}

#endif