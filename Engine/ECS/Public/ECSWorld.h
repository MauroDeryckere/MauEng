#ifndef MAUENG_ECSWORLD_H
#define MAUENG_ECSWORLD_H

#include "EntityID.h"
#include <memory>
#include <concepts>

#include "Asserts/Asserts.h"

#include "EnttImpl.h"

#include "View.h"
#include "Group.h"

namespace MauEng
{
	class Entity;
}

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

#pragma region Entities
		// Create an entity and add it to the ECS
		[[nodiscard]] Entity CreateEntity() & noexcept;

		// Destroy an Entity & remove it from the ECS
		void DestroyEntity(Entity entity) & noexcept;
		// Destroy an Entity & remove it from the ECS
		void DestroyEntity(EntityID id) & noexcept;

		// Checks if the entity is valid
		[[nodiscard]] bool IsValid(Entity entity) const& noexcept;
		// Checks if the entity is valid
		[[nodiscard]] bool IsValid(EntityID id) const& noexcept;
#pragma endregion

#pragma region Components
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
		 * @brief Remove a component from an entity.
		 * @tparam ComponentType Type of component to construct.
		 * @param id to remove the component from.
		 * @return If the component was removed.
		*/
		template <typename ComponentType>
		bool RemoveComponent(EntityID id) & noexcept
		{
			ME_ASSERT(IsValid(id));
			ME_ASSERT(HasComponent<ComponentType>(id));
			return m_pImpl->RemoveComponent<ComponentType>(id);
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
		 * @brief
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
		 * @brief
		 * @tparam ComponentType Type of component to construct.
		 * @param id entity to check.
		 * @return Component or nullptr if entity does not have the component
		*/
		template<typename ComponentType>
		[[nodiscard]] ComponentType const* TryGetComponent(EntityID id)const & noexcept
		{
			return m_pImpl->TryGetComponent<ComponentType>(id);
		}
#pragma endregion

#pragma region ViewsAndGroups
#pragma region Views
		template<typename... ComponentTypes, typename... ExcludeTypes>
		[[nodiscard]] auto View(ExcludeType<ExcludeTypes...> exclude = ExcludeType{})& noexcept
		{
			auto view{ m_pImpl->View<ComponentTypes...>(exclude) };
			return ViewWrapper<ComponentTypes...>(view);
		}
		template<typename... ComponentTypes, typename... ExcludeTypes>
		[[nodiscard]] auto View(ExcludeType<ExcludeTypes...> exclude = ExcludeType{})const& noexcept
		{
			auto view{ m_pImpl->View<ComponentTypes...>(exclude) };
			return ViewWrapper<ComponentTypes...>(view);
		}
#pragma endregion
#pragma region Groups
		template<typename... Owned, typename... Get, typename... ExcludeTypes>
		[[nodiscard]] auto Group(GetType<Get...> get = GetType{}, ExcludeType<ExcludeTypes...> exclude = ExcludeType{}) & noexcept
		{
			auto group{ m_pImpl->Group<Owned...>(get, exclude) };
			return GroupWrapper<decltype(group), Owned..., Get...>(group);
		} 

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
