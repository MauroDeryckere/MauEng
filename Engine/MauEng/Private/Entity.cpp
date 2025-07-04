#include "Entity.h"

#include "Asserts/Asserts.h"
#include "Components/CStaticMesh.h"
namespace MauEng
{
	Entity::Entity(ECS::ECSWorld* pWorld, ECS::EntityID id) :
		m_pECSWorld{ pWorld },
		m_ID{ id }
	{}

	void Entity::Destroy() noexcept
	{
		if (m_pECSWorld->HasComponent<CStaticMesh>(m_ID))
		{
			m_pECSWorld->RemoveComponent<CStaticMesh>(m_ID);
		}

		m_pECSWorld->DestroyEntity(m_ID);
	}
}
