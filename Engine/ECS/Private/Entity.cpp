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

	Entity::operator bool() const noexcept
	{
		return m_pECSWorld && m_pECSWorld->IsValid(m_ID);
	}
}
