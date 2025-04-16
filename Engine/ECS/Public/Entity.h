#ifndef MAUENG_ENTITY_H
#define MAUENG_ENTITY_H

#include "ECSWorld.h"
#include "EntityID.h"

namespace MauEng
{
	namespace ECS
	{
		class ECSWorld;
	}

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
		 * @brief Add a component to the entity.
		 * @tparam ComponentType Type of component to construct.
		 * @tparam Args Argument types to construct component.
		 * @param args args to construct the component.
		 * @return Added component by reference.
		*/
		template<typename ComponentType, typename... Args>
		ComponentType& AddComponent(Args&&... args) noexcept;

		/**
		 * @brief Remove a component from the entity.
		 * @tparam ComponentType Type of component to construct.
		 * @return If the component was removed.
		*/
		template<typename ComponentType>
		bool RemoveComponent() noexcept;

		/**
		 * @brief Get a component from the entity.
		 * @tparam ComponentType Type of component to get.
		 * @return Component.
		 * @note component must be added before it is safe to call this.
		*/
		template<typename ComponentType>
		[[nodiscard]] ComponentType const& GetComponent() const noexcept;
		/**
		 * @brief Get a component from the entity.
		 * @tparam ComponentType Type of component to get.
		 * @return Component.
		 * @note component must be added before it is safe to call this.
		*/
		template<typename ComponentType>
		[[nodiscard]] ComponentType& GetComponent() noexcept;

		/**
		 * @brief Check if the entity has a specific component.
		 * @tparam ComponentType Type of component to construct.
		 * @return If the entity has the component.
		*/
		template<typename ComponentType>
		[[nodiscard]] bool HasComponent() const noexcept;

#pragma region operators
		operator bool() const noexcept { return m_pECSWorld && m_pECSWorld->IsValid(m_ID); }

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