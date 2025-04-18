#ifndef MAUENGECS_GROUP_H
#define MAUENGECS_GROUP_H

#include "EntityID.h"
#include <memory>
#include <concepts>
#include <execution>

#include "Asserts/Asserts.h"
#include "EnttImpl.h"
#include "View.h"

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

		template<typename ComponentType>
		[[nodiscard]] ComponentType& Get(EntityID id) const noexcept
		{
			return m_Group.template get<ComponentType>(static_cast<InternalEntityType>(id));
		}
		template<typename ComponentType>
		[[nodiscard]] ComponentType* TryGet(EntityID id) const noexcept
		{
			return m_Group.template try_get<ComponentType>(static_cast<InternalEntityType>(id));
		}

		[[nodiscard]] bool Contains(EntityID id) const noexcept
		{
			return m_Group.contains(static_cast<InternalEntityType>(id));
		}
		template<typename ComponentType>
		[[nodiscard]] bool HasComponent(EntityID id) const noexcept
		{
			return m_Group.template contains<ComponentType>(static_cast<InternalEntityType>(id));
		}

		// add a else if constexpr for a func() with comps
		template<typename Func>
			requires (std::is_invocable_v<Func, EntityID, EntityID>)
		void Sort(Func&& func) noexcept
		{
			m_Group.sort([&](EntityID lhs, EntityID rhs) 
				{
					return func(lhs, rhs);
				});
		}

		template<typename ComponentType, typename... Args>
		void Emplace(EntityID id, Args&&... args) noexcept
			requires std::constructible_from<ComponentType, Args...>
		{
			m_Group.template emplace<ComponentType>(static_cast<InternalEntityType>(id), std::forward<Args>(args)...);
		}

		template<typename ComponentType>
		void Remove(EntityID id) noexcept
		{
			ME_ASSERT(HasComponent<ComponentType>(id));
			m_Group.template remove<ComponentType>(static_cast<InternalEntityType>(id));
		}

		[[nodiscard]] EntityID Front() const noexcept
		{
			ME_ASSERT(!Empty());
			return static_cast<EntityID>(*begin());
		}

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