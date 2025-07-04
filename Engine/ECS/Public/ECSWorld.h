#ifndef MAUENG_ECSWORLD_H
#define MAUENG_ECSWORLD_H

#include "EntityID.h"
#include <memory>
#include <concepts>

#include "CoreServiceLocator.h"
#include "Asserts/Asserts.h"

#include "EnttImpl.h"

#include "View.h"
#include "Group.h"

namespace MauEng::ECS
{
	class ECSWorld final
	{
	public:
		ECSWorld();
		~ECSWorld();

		ECSWorld(ECSWorld const&) = delete;
		ECSWorld(ECSWorld&&) = delete;
		ECSWorld& operator=(ECSWorld const&) = delete;
		ECSWorld& operator=(ECSWorld&&) = delete;

		template<typename... ComponentTypes>
		void Compact()& noexcept
		{
			m_pImpl->Compact<ComponentTypes...>();
		}

		template<typename... ComponentTypes>
		[[nodiscard]] bool IsOwned() const& noexcept
		{
			return m_pImpl->IsOwned<ComponentTypes...>();
		}

#pragma region Entities
		// Create an entity and add it to the ECS
		[[nodiscard]] EntityID CreateEntity() & noexcept;

		// Destroy an Entity & remove it from the ECS
		void DestroyEntity(EntityID id) & noexcept;

		// Checks if the entity is valid
		[[nodiscard]] bool IsValid(EntityID id) const& noexcept;

		/**
		 * @brief Remove all components in the registry of a given type.
		 * @tparam ComponentType Component type to clear.
		 * @tparam ComponentTypes Additional Component types to clear.
		 */
		template <typename ComponentType, typename... ComponentTypes>
		void Clear()& noexcept
		{
			m_pImpl->Clear<ComponentType, ComponentTypes...>();
		}

		/**
		 * @brief Check if an entity has all the listed components.
		 * @tparam ComponentTypes Component types to check.
		 * @param id to check.
		 * @return If the entity has all the components.
		 */
		template <typename... ComponentTypes>
		[[nodiscard]] bool HasAllOfComponents(EntityID id) const& noexcept
		{
			ME_ASSERT(IsValid(id));
			return m_pImpl->HasAllOfComponents<ComponentTypes...>(id);
		}
		/**
		 * @brief Check if an entity has any of the listed components.
		 * @tparam ComponentTypes Component types to check.
		 * @param id to check.
		 * @return If the entity has any of the components.
		 */
		template <typename... ComponentTypes>
		[[nodiscard]] bool HasAnyOfComponents(EntityID id) const& noexcept
		{
			ME_ASSERT(IsValid(id));
			return m_pImpl->HasAnyOfComponents<ComponentTypes...>(id);
		}

		/**
		 * @brief Remove components from the entity.
		 * @tparam FirstType Type of component to remove.
		 * @tparam ComponentTypes Other types of component to remove.
		 * @param id id to remove the component from.
		 * @note Remove checks if the comp exists first, erase does not.
		*/
		template<typename FirstType, typename... ComponentTypes>
		void Erase(EntityID id) noexcept
		{
			ME_ASSERT(IsValid(id));
			ME_ASSERT(HasComponent<FirstType>(id));
			ME_ASSERT(HasAllOfComponents<ComponentTypes...>(id));

			m_pImpl->Erase<FirstType, ComponentTypes...>(id);
		}

		/**
		 * @brief Remove components from the entity.
		 * @tparam FirstType Type of component to remove.
		 * @tparam ComponentTypes Other types of component to remove.
		 * @tparam Iterator iterator type.
		 * @param begin start of the range to remove components from.
		 * @param end end of the range to remove components from.
		 * @note Remove checks if the comp exists first, erase does not.
		 * @warning There are no asserts here to check if the componennt exists when asserts are enabled
		*/
		template<typename FirstType, typename... ComponentTypes, typename Iterator>
		void Erase(Iterator begin, Iterator end) noexcept
		{
			m_pImpl->Erase<FirstType, ComponentTypes...>(begin, end);
		}

		/**
		 * @brief Add component to the entity.
		 * @tparam ComponentType Type of component to insert.
		 * @tparam It iterator type.
		 * @param begin start of the range to add component to.
		 * @param end end of the range to add component to.
		 * @param component component to add
		*/
		template<typename ComponentType, typename It>
		void Insert(It begin, It end, ComponentType const& component)
		{
			m_pImpl->Insert(begin, end, component);
		}

#pragma endregion

#pragma region Components
		template<typename ComponentType, typename Func>
		void ConnectOnDestroy(Func&& callback)
		{
			m_pImpl->ConnectOnDestroy<ComponentType>(std::forward<Func>(callback));
		}
		template<typename ComponentType>
		void RegisterPreRemoveCallback(std::function<void(ComponentType const&)>&& callback)
		{
			m_pImpl->RegisterPreRemoveCallback<ComponentType>(std::move(callback));
		}

		template<typename ComponentType>
		[[nodiscard]] std::size_t ComponentCount() const& noexcept
		{
			return m_pImpl->ComponentCount<ComponentType>();
		}

		/**
		 * @brief Add a component to the ECS.
		 * @tparam ComponentType Type of component to construct.
		 * @tparam Args Argument types to construct component.
		 * @param id to add the component to.
		 * @param args to construct the component.
		 * @return Added component by reference.
		 */
		template<typename ComponentType, typename... Args>
			requires std::is_constructible_v<ComponentType, Args...>
		ComponentType& AddComponent(EntityID id, Args&&... args) & noexcept
		{
			ME_ASSERT(IsValid(id));
			ME_ASSERT(not HasComponent<ComponentType>(id));
			return m_pImpl->AddComponent<ComponentType>(id, std::forward<Args>(args)...);
		}

		/**
		 * @brief Get a component from the ECS.
		 * @tparam ComponentType Type of component to get.
		 * @param id id to get the component from.
		 * @return Component.
		 * @note component must be added before it is safe to call this.
		*/
		template <typename ComponentType>
		[[nodiscard]] ComponentType const& GetComponent(EntityID id) const& noexcept
		{
			ME_ASSERT(IsValid(id));
			ME_ASSERT(HasComponent<ComponentType>(id));
			return m_pImpl->GetComponent<ComponentType>(id);
		}

		/**
		 * @brief Get a component from the ECS.
		 * @tparam ComponentType Type of component to get.
		 * @param id id to get the component from.
		 * @return Component.
		 * @note component must be added before it is safe to call this.
		*/
		template <typename ComponentType>
		[[nodiscard]] ComponentType& GetComponent(EntityID id) & noexcept
		{
			ME_ASSERT(IsValid(id));
			ME_ASSERT(HasComponent<ComponentType>(id));
			return m_pImpl->GetComponent<ComponentType>(id);
		}

		/**
		 * @brief Remove components from the entity.
		 * @tparam FirstComponentType Type of component to remove.
		 * @tparam OtherComponentTypes Other types to remove.
		 * @param id to remove the components from.
		 * @return If all listed components were removed.
		 * @note Remove checks if the comp exists first, erase does not.
		*/
		template <typename FirstComponentType, typename... OtherComponentTypes>
		bool RemoveComponent(EntityID id) & noexcept
		{
			ME_ASSERT(IsValid(id));
			return m_pImpl->RemoveComponent<FirstComponentType, OtherComponentTypes... >(id);
		}

		/**
		 * @brief Remove components from the entity.
		 * @tparam FirstComponentType Type of component to remove.
		 * @tparam OtherComponentTypes Other types to remove.
		 * @tparam Iterator iterator type.
		 * @param begin begin of the range to remove.
		 * @param end end of the range to remove.
		 * @return If all listed components were removed.
		 * @note Remove checks if the comp exists first, erase does not.
		*/
		template <typename FirstComponentType, typename... OtherComponentTypes, typename Iterator>
		[[nodiscard]] bool RemoveComponent(Iterator begin, Iterator end) noexcept
		{
			return m_pImpl->RemoveComponent<FirstComponentType, OtherComponentTypes...>(begin, end);
		}

		/**
		 * @brief Check if an entity has a specific component.
		 * @tparam ComponentType Type of component to construct.
		 * @param id entity to check.
		 * @return If the entity has the component.
		*/
		template<typename ComponentType>
		[[nodiscard]] bool HasComponent(EntityID id) const& noexcept
		{
			ME_ASSERT(IsValid(id));
			return m_pImpl->HasComponent<ComponentType>(id);
		}

		/**
		 * @brief try to get a component from the ECS.
		 * @tparam ComponentType Type of component to construct.
		 * @param id entity to check.
		 * @return Component or nullptr if entity does not have the component
		*/
		template<typename ComponentType>
		[[nodiscard]] ComponentType* TryGetComponent(EntityID id)& noexcept
		{
			return m_pImpl->TryGetComponent<ComponentType>(id);
		}

		/**
		 * @brief try to get a component from the ECS.
		 * @tparam ComponentType Type of component to construct.
		 * @param id entity to check.
		 * @return Component or nullptr if entity does not have the component
		*/
		template<typename ComponentType>
		[[nodiscard]] ComponentType const* TryGetComponent(EntityID id)const & noexcept
		{
			return m_pImpl->TryGetComponent<ComponentType>(id);
		}

		/**
		 * @brief Replace a component in the ECS.
		 * @tparam ComponentType Type of component to construct.
		 * @tparam Args Argument types to construct component.
		 * @param id to add the component to.
		 * @param args to construct the component.
		 * @return Added component by reference.
		*/
		template<typename ComponentType, typename... Args>
			requires std::is_constructible_v<ComponentType, Args...>
		ComponentType& ReplaceComponent(EntityID id, Args&&... args) noexcept
		{
			ME_ASSERT(IsValid(id));
			ME_ASSERT(HasComponent<ComponentType>(id));

			return m_pImpl->ReplaceComponent<ComponentType>(id, std::forward<Args>(args)...);
		}

		/**
		 * @brief Add or replace a component in the ECS.
		 * @tparam ComponentType Type of component to construct.
		 * @tparam Args Argument types to construct component.
		 * @param id to add the component to.
		 * @param args to construct the component.
		 * @return Added component by reference.
		*/
		template<typename ComponentType, typename... Args>
			requires std::is_constructible_v<ComponentType, Args...>
		ComponentType& AddOrReplaceComponent(EntityID id, Args&&... args) & noexcept
		{
			ME_ASSERT(IsValid(id));
			return m_pImpl->AddOrReplaceComponent<ComponentType>(id, std::forward<Args>(args)...);
		}

		/**
		 * @brief Get or emplace a component in the ECS.
		 * @tparam ComponentType Type of component to construct.
		 * @tparam Args Argument types to construct component.
		 * @param id to add the component to.
		 * @param args to construct the component.
		 * @return Added component by reference.
		*/
		template<typename ComponentType, typename... Args>
			requires std::is_constructible_v<ComponentType, Args...>
		ComponentType& GetOrEmplaceComponent(EntityID id, Args&&... args) & noexcept
		{
			ME_ASSERT(IsValid(id));
			return m_pImpl->GetOrEmplaceComponent<ComponentType>(id, std::forward<Args>(args)...);
		}
		
		/**
		 * @brief Sort a specific component type in the ECS.
		 * @tparam ComponentType Type of component to sort.
		 * @tparam Compare Type of comparison object.
		 * @param compare comparison object.
		*/
		template<typename ComponentType, typename Compare>
			requires std::invocable<Compare, ComponentType const&, ComponentType const&>
				  || std::invocable<Compare, EntityID, EntityID>
		void Sort(Compare&& compare) & noexcept
		{
			if (IsOwned<ComponentType>())
			{
				// Could get the group and do the sort there automatically too
				ME_LOG(MauCor::ELogPriority::Error, LogEngine, "Can not sort, trying to sort owned components (use group sort)");
				return;
			}

			if constexpr (std::invocable<Compare, EntityID, EntityID>)
			{
				m_pImpl->Sort<ComponentType>([&](InternalEntityType lhs, InternalEntityType rhs) { return compare(static_cast<EntityID>(lhs), static_cast<EntityID>(rhs)); });
			}
			else if constexpr (std::invocable<Compare, ComponentType const&, ComponentType const&>)
			{
				m_pImpl->Sort<ComponentType>(std::forward<Compare>(compare));
			}
		}

		// @brief Sort 2 component types to be more cache efficient in the registry (e.g Transform, Mesh, Transform, Mesh and so on).
		template<typename ComponentType1, typename ComponentType2>
		void Sort() & noexcept
		{
			if (IsOwned<ComponentType1, ComponentType2>())
			{
				// Could get the group and do the sort there automatically too
				ME_LOG(MauCor::ELogPriority::Error, LogEngine, "Can not sort, trying to sort owned components (use group sort)");
				return;
			}

			m_pImpl->Sort<ComponentType1, ComponentType2>();
		}

#pragma endregion

#pragma region ViewsAndGroups
#pragma region Views
		/**
		 * @brief Create a view to iterate over entities with the specified components.
		 * @tparam ComponentTypes Type of component to view.
		 * @tparam ExcludeTypes Type of component to exclude from theview.
		 * @param exclude components to exclude from the view.
		 * @return the view.
		*/
		template<typename... ComponentTypes, typename... ExcludeTypes>
		[[nodiscard]] auto View(ExcludeType<ExcludeTypes...> exclude = ExcludeType{})& noexcept
		{
			auto view{ m_pImpl->View<ComponentTypes...>(exclude) };
			return ViewWrapper<ComponentTypes...>(view);
		}

		/**
		 * @brief Create a view to iterate over entities with the specified components.
		 * @tparam ComponentTypes Type of component to view.
		 * @tparam ExcludeTypes Type of component to exclude from theview.
		 * @param exclude components to exclude from the view.
		 * @return the view.
		*/
		template<typename... ComponentTypes, typename... ExcludeTypes>
		[[nodiscard]] auto View(ExcludeType<ExcludeTypes...> exclude = ExcludeType{})const& noexcept
		{
			auto view{ m_pImpl->View<ComponentTypes...>(exclude) };
			return ViewWrapper<ComponentTypes...>(view);
		}
#pragma endregion
#pragma region Groups
		/**
		 * @brief Create a group to iterate over entities with the specified components.
		 * @tparam Owned Type of component to group (owned).
		 * @tparam Get Type of component to group (non-owned, just for viewin).
		 * @tparam ExcludeTypes Type of component to exclude from the group.
		 * @param get components to get in group.
		 * @param exclude components to exclude from group.
		 * @return the group.
		*/
		template<typename... Owned, typename... Get, typename... ExcludeTypes>
		[[nodiscard]] auto Group(GetType<Get...> get = GetType{}, ExcludeType<ExcludeTypes...> exclude = ExcludeType{}) & noexcept
		{
			auto group{ m_pImpl->Group<Owned...>(get, exclude) };
			return GroupWrapper<decltype(group), Owned..., Get...>(group);
		} 

		/**
		 * @brief Create a group to iterate over entities with the specified components.
		 * @tparam Owned Type of component to group (owned).
		 * @tparam Get Type of component to group (non-owned, just for viewin).
		 * @tparam ExcludeTypes Type of component to exclude from the group.
		 * @param get components to get in group.
		 * @param exclude components to exclude from group.
		 * @return the group.
		*/
		template<typename... Owned, typename... Get, typename... ExcludeTypes>
		[[nodiscard]] auto Group(GetType<Get...> get = GetType{}, ExcludeType<ExcludeTypes...> exclude = ExcludeType{})const& noexcept
		{
			auto group{ m_pImpl->Group<Owned...>(get, exclude) };
			return GroupWrapper<decltype(group), Owned..., Get...>(group);
		}
#pragma endregion
#pragma endregion

	private:
		std::unique_ptr<ECSImpl> m_pImpl;
	};

}

#endif
