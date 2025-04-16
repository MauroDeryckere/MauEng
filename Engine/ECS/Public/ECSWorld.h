#ifndef MAUENG_ECSWORLD_H
#define MAUENG_ECSWORLD_H

#include "Entity.h"

#include <memory>
namespace MauEng
{
namespace ECS
{
	struct ECSImpl;

	class ECSWorld final
	{
	public:
		ECSWorld();
		~ECSWorld();

		template<typename ComponentType, typename... Args>
		ComponentType& AddComponent(EntityID id, Args&&... args) noexcept;
		template<typename ComponentType, typename... Args>
		ComponentType& AddComponent(Entity const entity, Args&&... args) noexcept;

		template<typename ComponentType>
		[[nodiscard]] bool HasComponent(Entity const entity) const noexcept;

		Entity CreateEntity() noexcept;

	private:
		std::unique_ptr<ECSImpl> m_pImpl;
	};

}
}

#endif