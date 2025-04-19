#ifndef MAUENGECS_GROUP_H
#define MAUENGECS_GROUP_H

#include "EntityID.h"
#include <memory>
#include <concepts>
#include <execution>

#include "Asserts/Asserts.h"
#include "EnttImpl.h"
#include "View.h"

#include <algorithm>

namespace MauEng::ECS
{
	template<typename... GetTypes>
	using GetType = entt::get_t<GetTypes...>;

	template<typename GroupT, typename... ComponentTypes>
	class GroupWrapper final
	{
	public:
		using GroupType = GroupT;

		explicit GroupWrapper(GroupType& group)
			: m_Group{ group } {
			ME_ASSERT(m_Group);
		}
		~GroupWrapper() = default;

		GroupWrapper(GroupWrapper const&) = default;
		GroupWrapper(GroupWrapper&&) = default;
		GroupWrapper& operator=(GroupWrapper const&) = default;
		GroupWrapper& operator=(GroupWrapper&&) = default;

		/**
		 * @brief Iterate over all entities with the given components
		 * @tparam Func Function type (usually automatically deduced)
		 * @tparam ExecPolicy Execution policy when looping over elements multithreaded (usually automatically deduced)
		 * @param func Function to execute for each entity
		 * @param policy Policy to multithread with
		*/
		template<typename Func, typename ExecPolicy = std::execution::sequenced_policy>
			requires ( std::is_invocable_v<Func, ComponentTypes&...>
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
						if constexpr (sizeof...(ComponentTypes) > 1)
						{
							static_assert(std::is_same_v<
								decltype(m_Group.template get<ComponentTypes...>(InternalEntityType{})),
								std::tuple<ComponentTypes&...>
							> , "Group::get<ComponentTypes...> must return a tuple of references");

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
								m_Group.template get<ComponentTypes...>(entity)
							);
						}
						else
						{
							if constexpr (std::is_invocable_v<Func, EntityID, ComponentTypes&...>)
							{
								func(static_cast<EntityID>(entity), m_Group.template get<ComponentTypes...>(entity));
							}
							else if constexpr (std::is_invocable_v<Func, ComponentTypes&...>)
							{
								func(m_Group.template get<ComponentTypes...>(entity));
							}
							else if constexpr (std::is_invocable_v<Func>)
							{
								func();
							}
						}

					} };

				std::for_each(policy, m_Group.begin(), m_Group.end(), parallelFuncCall);
			}
			else
			{
				if constexpr (std::is_invocable_v<Func, EntityID, ComponentTypes&...>)
				{
					m_Group.each([&](InternalEntityType id, ComponentTypes&... comps)
						{
							func(static_cast<EntityID>(id), comps...);
						});
				}
				else if constexpr (std::is_invocable_v<Func, ComponentTypes&...>)
				{
					m_Group.each([&](ComponentTypes&... comps)
						{
							func(comps...);
						});
				}
				else if constexpr (std::is_invocable_v<Func>)
				{
					m_Group.each([&](ComponentTypes&... comps)
						{
							func();
						});
				}
			}
		}

		/**
		 * @brief Sorts the group by the given component types.
		 * @tparam FirstComponentType First component type to sort.
		 * @tparam OtherComponentTypes Optional other component types to sort.
		 * @tparam Compare Type of callable comparison object.
		 * @tparam SortAlgo Type of callable sorting object.
		 * @tparam Args Additional args to pass to sorting obj
		 * @param compare comparison object.
		 * @param algo sorting algorithm object.
		 * @param args Additional args to pass to sorting obj.
		 * @warning group must own the components you sort
		*/
		template<typename FirstComponentType, typename... OtherComponentTypes, typename Compare, typename SortAlgo = entt::std_sort, typename... Args>
		void Sort(Compare&& compare, SortAlgo&& algo = SortAlgo{}, Args&&... args) noexcept
			requires
			(  sizeof...(OtherComponentTypes) == 0 && 
			   std::is_invocable_v<Compare, FirstComponentType const&, FirstComponentType const&> )
			|| std::is_invocable_v<Compare,
									std::tuple<FirstComponentType const&, OtherComponentTypes const&...> const&,
									std::tuple<FirstComponentType const&, OtherComponentTypes const&...> const&>
		{
			m_Group.template sort<FirstComponentType, OtherComponentTypes...>
			(
				std::forward<Compare>(compare),
				std::forward<SortAlgo>(algo),
				std::forward<Args>(args)...
			);
		}

		/**
		 * @brief Sorts the group by the given component types.
		 * @tparam FirstComponentType First component type to sort.
		 * @tparam OtherComponentTypes Optional other component types to sort.
		 * @tparam SortAlgo Type of callable sorting object.
		 * @tparam Args Additional args to pass to sorting obj
		 * @param algo sorting algorithm object.
		 * @param args Additional args to pass to sorting obj.
		 * @warning group must own the components you sort
		*/
		template<typename FirstComponentType, typename... OtherComponentTypes, typename SortAlgo = entt::std_sort, typename... Args>
		void Sort(SortAlgo&& algo = SortAlgo{}, Args&&... args) noexcept
			requires (requires(FirstComponentType const& a, FirstComponentType const& b) { a < b; }) &&
					 (requires(OtherComponentTypes const&... rest) { (... && (rest < rest)); })
		{
			m_Group.template sort<FirstComponentType, OtherComponentTypes...>
				(
					std::forward<SortAlgo>(algo),
					std::forward<Args>(args)...
				);
		}

		/**
		  * @brief Get component(s) from an entity in the group
		  * @tparam ComponentTs Function type (usually automatically deduced)
		  * @param id entity to get the components from
		  * @return the component or a tuple with returned components
		  * @warning An entity should own the component before using a get.
		*/
		template<typename... ComponentTs>
		[[nodiscard]] auto Get(EntityID id) const noexcept
		{
			ME_ASSERT(Contains(id));
			return m_Group.template get<ComponentTs...>(static_cast<InternalEntityType>(id));
		}
		/**
		  * @brief Get all components from an entity in the group, that are owned or observed by the group
s		  * @param id entity to get the components from
		  * @return the component or a tuple with returned components
		*/
		[[nodiscard]] auto Get(EntityID id) const noexcept
		{
			ME_ASSERT(Contains(id));
			return m_Group.get(static_cast<InternalEntityType>(id));
		}

		// Does the group contain this entity?
		[[nodiscard]] bool Contains(EntityID id) const noexcept
		{
			return m_Group.contains(static_cast<InternalEntityType>(id));
		}

		// First entity in the group
		[[nodiscard]] EntityID Front() const noexcept
		{
			ME_ASSERT(!Empty());
			return static_cast<EntityID>(*begin());
		}

		// Last entity in the group
		[[nodiscard]] EntityID Back() const noexcept
		{
			ME_ASSERT(!Empty());
			return static_cast<EntityID>(*rbegin());
		}

		[[nodiscard]] bool Empty() const noexcept { return m_Group.empty(); }
		[[nodiscard]] std::size_t Size() const noexcept { return m_Group.size(); }

		[[nodiscard]] auto begin() const noexcept { return m_Group.begin(); }
		[[nodiscard]] auto cbegin() const noexcept { return m_Group.cbegin(); }
		[[nodiscard]] auto rbegin() const noexcept { return m_Group.rbegin(); }
		[[nodiscard]] auto crbegin() const noexcept { return m_Group.crbegin(); }

		[[nodiscard]] auto end() const noexcept { return m_Group.end(); }
		[[nodiscard]] auto cend() const noexcept { return m_Group.cend(); }
		[[nodiscard]] auto rend() const noexcept { return m_Group.rend(); }
		[[nodiscard]] auto crend() const noexcept { return m_Group.crend(); }

	private:
		GroupType m_Group;
	};
}

#endif