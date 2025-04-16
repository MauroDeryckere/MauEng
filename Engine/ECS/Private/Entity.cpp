#include "Entity.h"

#include "ECSWorld.h"

namespace MauEng
{
	Entity::Entity(ECS::ECSWorld* pWorld, ECS::EntityID id) :
		m_pECSWorld{ pWorld },
		m_ID{ id }
	{}

	template<typename ComponentType, typename... Args>
	ComponentType& Entity::AddComponent(Args&&... args) noexcept
	{
		return m_pECSWorld->AddComponent<ComponentType>(m_ID, std::forward<Args>(args)...);
	}
}
