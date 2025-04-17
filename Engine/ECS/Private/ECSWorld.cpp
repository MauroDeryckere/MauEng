#include "ECSWorld.h"

#include "EnttImpl.h"
#include "Entity.h"

#include "Asserts/Asserts.h"

namespace MauEng::ECS
{
	ECSWorld::ECSWorld() :
		m_pImpl{ std::make_unique<ECSImpl>() }
	{
	}

	ECSWorld::~ECSWorld()
	{
		m_pImpl = nullptr;
	}

	Entity ECSWorld::CreateEntity()& noexcept
	{
		return Entity{ this, m_pImpl->CreateEntity() };
	}

	void ECSWorld::DestroyEntity(Entity entity)& noexcept
	{
		ME_ASSERT(IsValid(entity));
		DestroyEntity(entity.ID());
		entity.m_ID = NULL_ENTITY_ID;
	}

	void ECSWorld::DestroyEntity(EntityID id)& noexcept
	{
		ME_ASSERT(IsValid(id));
		m_pImpl->DestroyEntity(id);
	}

	bool ECSWorld::IsValid(Entity entity) const & noexcept
	{
		return m_pImpl->IsValid(entity.ID());
	}
	bool ECSWorld::IsValid(EntityID id) const & noexcept
	{
		return m_pImpl->IsValid(id);
	}
}
	