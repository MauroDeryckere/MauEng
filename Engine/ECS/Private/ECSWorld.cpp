#include "ECSWorld.h"

#include "EnttImpl.h"
#include "Entity.h"

namespace MauEng::ECS
{
	ECSWorld::ECSWorld() : 
		m_pImpl{ std::make_unique<ECSImpl>() }
	{ }

	ECSWorld::~ECSWorld()
	{
		m_pImpl = nullptr;
	}

	Entity ECSWorld::CreateEntity() noexcept
	{
		return Entity{ this, m_pImpl->CreateEntity() };
	}

	template <typename ComponentType, typename ... Args>
	ComponentType& ECSWorld::AddComponent(EntityID id, Args&&... args) noexcept
	{
		return m_pImpl->AddComponent<ComponentType>(id, std::forward<Args>(args)...);
	}

	template <typename ComponentType, typename ... Args>
	ComponentType& ECSWorld::AddComponent(Entity const entity, Args&&... args) noexcept
	{
		return m_pImpl->AddComponent<ComponentType>(entity.ID(), std::forward<Args>(args)...);
	}

	template<typename ComponentType>
	bool ECSWorld::HasComponent(Entity const entity) const noexcept
	{
		return m_pImpl->HasComponent<ComponentType>(entity.ID());
	}
}
