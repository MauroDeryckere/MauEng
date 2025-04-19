#ifndef MAUENG_ENTITY_H
#define MAUENG_ENTITY_H

#include "ECSWorld.h"
#include "EntityID.h"

namespace MauEng
{
	class Entity final
	{
	public:
		// Note: creating an entity here dooes not add it to the ECS World, create the entity via the CreateEntity function to do that
		Entity() = default;
		// Note: creating an entity here dooes not add it to the ECS World, create the entity via the CreateEntity function to do that
		Entity(ECS::ECSWorld* pWorld, ECS::EntityID id);
		~Entity() = default;

		Entity(Entity const&) = default;
		Entity(Entity&&) = default;
		Entity& operator=(Entity const&) = default;
		Entity& operator=(Entity&&) = default;

		// Destroy the entity (remove from the ECS world that it lives in)
		void Destroy() noexcept;

		// Returns the underlying entity ID
		[[nodiscard]] ECS::EntityID ID() const noexcept { return m_ID; }

		/**
		 * @brief Check if the entity has all the listed components.
		 * @tparam ComponentTypes Component types to check.
		 * @return If the entity has all the components.
		 */
		template <typename... ComponentTypes>
		[[nodiscard]] bool HasAllOfComponents() const& noexcept
		{
			return m_pECSWorld->HasAllOfComponents<ComponentTypes...>(m_ID);
		}
		/**
		 * @brief Check if the entity has any of the listed components.
		 * @tparam ComponentTypes Component types to check.
		 * @return If the entity has any of the components.
		 */
		template <typename... ComponentTypes>
		[[nodiscard]] bool HasAnyOfComponents() const& noexcept
		{
			return m_pECSWorld->HasAnyOfComponents<ComponentTypes...>(m_ID);
		}

#pragma region Components
		/**
		 * @brief Add a component to the entity.
		 * @tparam ComponentType Type of component to construct.
		 * @tparam Args Argument types to construct component.
		 * @param args args to construct the component.
		 * @return Added component by reference.
		*/
		template<typename ComponentType, typename... Args>
		ComponentType& AddComponent(Args&&... args) noexcept
		{
			return m_pECSWorld->AddComponent<ComponentType>(m_ID, std::forward<Args>(args)...);
		}

		/**
		 * @brief Remove components from the entity.
		 * @tparam FirstComponentType Type of component to remove.
		 * @tparam OtherComponentTypes Other types of component to remove.
		 * @return If all listed component were removed.
		 * @note Remove checks if the comp exists first, erase does not.
		*/
		template <typename FirstComponentType, typename... OtherComponentTypes>
		bool RemoveComponent() noexcept
		{
			return m_pECSWorld->RemoveComponent<FirstComponentType, OtherComponentTypes...>(m_ID);
		}

		/**
		 * @brief Remove components from the entity.
		 * @tparam FirstComponentType Type of component to remove.
		 * @tparam OtherComponentTypes Other types of component to remove.
		 * @note Remove checks if the comp exists first, erase does not.
		*/
		template<typename FirstComponentType, typename... OtherComponentTypes>
		void EraseComponent() noexcept
		{
			return m_pECSWorld->Erase<FirstComponentType, OtherComponentTypes...>(m_ID);
		}

		/**
		 * @brief Get a component from the entity.
		 * @tparam ComponentType Type of component to get.
		 * @return Component.
		 * @note component must be added before it is safe to call this.
		*/
		template<typename ComponentType>
		[[nodiscard]] ComponentType const& GetComponent() const noexcept
		{
			return m_pECSWorld->GetComponent<ComponentType>(m_ID);
		}

		/**
		 * @brief Get a component from the entity.
		 * @tparam ComponentType Type of component to get.
		 * @return Component.
		 * @note component must be added before it is safe to call this.
		*/
		template<typename ComponentType>
		[[nodiscard]] ComponentType& GetComponent() noexcept
		{
			return m_pECSWorld->GetComponent<ComponentType>(m_ID);
		}

		/**
		 * @brief Check if the entity has a specific component.
		 * @tparam ComponentType Type of component to construct.
		 * @return If the entity has the component.
		*/
		template<typename ComponentType>
		[[nodiscard]] bool HasComponent() const noexcept
		{
			return m_pECSWorld->HasComponent<ComponentType>(m_ID);
		}

		/**
		*@brief
		* @tparam ComponentType Type of component to construct.
		* @return Component or nullptr if entity does not have the component
		*/
		template<typename ComponentType>
		[[nodiscard]] ComponentType* TryGetComponent() noexcept
		{
			return m_pECSWorld->TryGetComponent<ComponentType>(m_ID);
		}

		/**
		*@brief
		* @tparam ComponentType Type of component to construct.
		* @return Component or nullptr if entity does not have the component
		*/
		template<typename ComponentType>
		[[nodiscard]] ComponentType const* TryGetComponent() const noexcept
		{
			return m_pECSWorld->TryGetComponent<ComponentType>(m_ID);
		}

		/**
		 * @brief Replace a component form the entity.
		 * @tparam ComponentType Type of component to construct.
		 * @tparam Args Argument types to construct component.
		 * @param args to construct the component.
		 * @return Added component by reference.
		*/
		template<typename ComponentType, typename... Args>
		ComponentType& ReplaceComponent(Args&&... args) noexcept
		{
			return m_pECSWorld->ReplaceComponent<ComponentType>(m_ID, std::forward<Args>(args)...);
		}

		/**
		 * @brief Get or emplace a component in the ECS.
		 * @tparam ComponentType Type of component to construct.
		 * @tparam Args Argument types to construct component.
		 * @param args to construct the component.
		 * @return Added component by reference.
		*/
		template<typename ComponentType, typename... Args>
		ComponentType& GetOrEmplaceComponent(Args&&... args) noexcept
		{
			return m_pECSWorld->GetOrEmplaceComponent<ComponentType>(m_ID, std::forward<Args>(args)...);
		}
		/**
		 * @brief Add or replace a component in the ECS.
		 * @tparam ComponentType Type of component to construct.
		 * @tparam Args Argument types to construct component.
		 * @param args to construct the component.
		 * @return Added component by reference.
		*/
		template<typename ComponentType, typename... Args>
		ComponentType& AddOrReplaceComponent(Args&&... args) noexcept
		{
			return m_pECSWorld->AddOrReplaceComponent<ComponentType>(m_ID, std::forward<Args>(args)...);
		}

#pragma endregion

#pragma region operators
		operator bool() const noexcept
		{
			return m_pECSWorld && m_pECSWorld->IsValid(m_ID);
		}

		[[nodiscard]] bool operator==(Entity const& other) const noexcept
		{
			return m_ID == other.m_ID
				&& m_pECSWorld == other.m_pECSWorld;
		}
		[[nodiscard]] bool operator!=(Entity const& other) const noexcept
		{
			return m_ID != other.m_ID
				|| m_pECSWorld != other.m_pECSWorld;
		}
#pragma endregion

	private:
		ECS::EntityID m_ID{ ECS::NULL_ENTITY_ID };

		friend class ECS::ECSWorld;
		// Reference to the ECS world
		ECS::ECSWorld* m_pECSWorld{ nullptr };
	};
}

#endif