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
		DestroyEntity(entity.ID());
		entity.m_ID = NULL_ENTITY_ID;
	}

	void ECSWorld::DestroyEntity(EntityID id)& noexcept
	{
		m_pImpl->DestroyEntity(id);
	}

	template <typename ComponentType, typename ... Args>
	requires std::is_constructible_v<ComponentType, Args...>
	ComponentType& ECSWorld::AddComponent(EntityID id, Args&&... args)& noexcept
	{
		ME_ASSERT(not HasComponent<ComponentType>(id));
		return m_pImpl->AddComponent<ComponentType>(id, std::forward<Args>(args)...);
	}
	template <typename ComponentType, typename ... Args>
	requires std::is_constructible_v<ComponentType, Args...>
	ComponentType& ECSWorld::AddComponent(Entity const entity, Args&&... args)& noexcept
	{
		ME_ASSERT(not HasComponent<ComponentType>(entity.ID()));
		return m_pImpl->AddComponent<ComponentType>(entity.ID(), std::forward<Args>(args)...);
	}

	template <typename ComponentType>
	ComponentType const& ECSWorld::GetComponent(EntityID id) const& noexcept
	{
		ME_ASSERT(HasComponent<ComponentType>(id));
		return m_pImpl->GetComponent<ComponentType>(id);
	}
	template <typename ComponentType>
	ComponentType const& ECSWorld::GetComponent(Entity const entity) const& noexcept
	{
		return GetComponent<ComponentType>(entity.ID());
	}
	template <typename ComponentType>
	ComponentType& ECSWorld::GetComponent(EntityID id)& noexcept
	{
		ME_ASSERT(HasComponent<ComponentType>(id));
		return m_pImpl->GetComponent<ComponentType>(id);
	}
	template <typename ComponentType>
	ComponentType& ECSWorld::GetComponent(Entity const entity)& noexcept
	{
		return GetComponent<ComponentType>(entity.ID());
	}

	template <typename ComponentType>
	bool ECSWorld::RemoveComponent(EntityID id)& noexcept
	{
		ME_ASSERT(HasComponent<ComponentType>(id));
		return m_pImpl->RemoveComponent<ComponentType>(id);
	}
	template <typename ComponentType>
	bool ECSWorld::RemoveComponent(Entity const entity)& noexcept
	{
		return RemoveComponent<ComponentType>(entity.ID());
	}

	template<typename ComponentType>
	bool ECSWorld::HasComponent(Entity const entity) const& noexcept
	{
		return m_pImpl->HasComponent<ComponentType>(entity.ID());
	}

	template <typename ComponentType>
	bool ECSWorld::HasComponent(EntityID id) const& noexcept
	{
		return m_pImpl->HasComponent<ComponentType>(id);
	}
}