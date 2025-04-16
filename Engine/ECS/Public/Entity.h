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
		Entity(ECS::ECSWorld* pWorld, ECS::EntityID id);
		~Entity() = default;

		Entity(Entity const&) = delete;
		Entity(Entity&&) = delete;
		Entity& operator=(Entity const&) = delete;
		Entity& operator=(Entity&&) = delete;

		[[nodiscard]] inline ECS::EntityID ID() const noexcept { return m_ID; }

		template<typename ComponentType, typename... Args>
		ComponentType& AddComponent(Args&&... args) noexcept;

		template<typename ComponentType>
		void RemoveComponent() noexcept;

		template<typename ComponentType>
		[[nodiscard]] ComponentType& GetComponent() const noexcept;
			
		template<typename ComponentType>
		[[nodiscard]] bool HasComponent() const noexcept;

	private:
		ECS::EntityID m_ID{ ECS::NULL_ENTITY_ID };

		// Reference to the ECS world
		ECS::ECSWorld* m_pECSWorld{ nullptr };

	};
}

#endif