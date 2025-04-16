#include "Entity.h"

#include "ECSWorld.h"
#include "Asserts/Asserts.h"

namespace MauEng
{
	Entity::Entity(ECS::ECSWorld* pWorld, ECS::EntityID id) :
		m_pECSWorld{ pWorld },
		m_ID{ id }
	{}

	void Entity::Destroy() noexcept
	{
		m_pECSWorld->DestroyEntity(*this);
	}

	template<typename ComponentType, typename... Args>
	ComponentType& Entity::AddComponent(Args&&... args) noexcept
	{
		return m_pECSWorld->AddComponent<ComponentType>(m_ID, std::forward<Args>(args)...);
	}

	template <typename ComponentType>
	void Entity::RemoveComponent() noexcept
	{
		m_pECSWorld->RemoveComponent<ComponentType>(m_ID);
	}

	template <typename ComponentType>
	ComponentType& Entity::GetComponent() const noexcept
	{
		return m_pECSWorld->GetComponent<ComponentType>(m_ID);;
	}

	template <typename ComponentType>
	bool Entity::HasComponent() const noexcept
	{
		return m_pECSWorld->HasComponent<ComponentType>(m_ID);
	}
}
