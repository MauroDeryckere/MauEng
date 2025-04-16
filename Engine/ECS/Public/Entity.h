#ifndef MAUENG_ENTITY_H
#define MAUENG_ENTITY_H

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
		Entity() = default;
		Entity(ECS::ECSWorld* pWorld, ECS::EntityID id);
		~Entity() = default;

		Entity(Entity const&) = default;
		Entity(Entity&&) = default;
		Entity& operator=(Entity const&) = default;
		Entity& operator=(Entity&&) = default;

		void Destroy() noexcept;

		[[nodiscard]] inline ECS::EntityID ID() const noexcept { return m_ID; }

		template<typename ComponentType, typename... Args>
		ComponentType& AddComponent(Args&&... args) noexcept;

		// ComponentType& AddOrReplaceComponent(Args&&... args)
		template<typename ComponentType>
		void RemoveComponent() noexcept;

		template<typename ComponentType>
		[[nodiscard]] ComponentType& GetComponent() const noexcept;
			
		template<typename ComponentType>
		[[nodiscard]] bool HasComponent() const noexcept;


#pragma region operators
		operator bool() const noexcept { return m_ID != ECS::NULL_ENTITY_ID; }

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