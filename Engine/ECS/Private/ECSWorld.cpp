#include "ECSWorld.h"

#include "EnttImpl.h"

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

	EntityID ECSWorld::CreateEntity()& noexcept
	{
		return m_pImpl->CreateEntity();
	}

	void ECSWorld::DestroyEntity(EntityID id)& noexcept
	{
		ME_ASSERT(IsValid(id));
		m_pImpl->DestroyEntity(id);
	}

	bool ECSWorld::IsValid(EntityID id) const & noexcept
	{
		return m_pImpl->IsValid(id);
	}
}
	