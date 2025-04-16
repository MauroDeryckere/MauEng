#ifndef MAUENG_ECSWORLD_H
#define MAUENG_ECSWORLD_H

#include "Entity.h"
#include <memory>
#include <concepts>

namespace MauEng::ECS
{
	struct ECSImpl;

	class ECSWorld final
	{
	public:
		ECSWorld();
		~ECSWorld();

		ECSWorld(ECSWorld const&) = delete;
		ECSWorld(ECSWorld&&) = delete;
		ECSWorld& operator=(ECSWorld const&) = delete;
		ECSWorld& operator=(ECSWorld&&) = delete;

		/**
		 * @brief Add a component to the ECS.
		 * @tparam ComponentType Type of component to construct.
		 * @tparam Args Argument types to construct component.
		 * @param id to add the component to.
		 * @param args to construct the component.
		 * @return Added component by reference.
		 */
		template<typename ComponentType, typename... Args>
		requires std::is_constructible_v<ComponentType, Args...>
		ComponentType& AddComponent(EntityID id, Args&&... args)& noexcept;
		/**
		 * @brief Add a component to the ECS.
		 * @tparam ComponentType Type of component to construct.
		 * @tparam Args Argument types to construct component.
		 * @param entity entity to add the component to.
		 * @param args args to construct the component.
		 * @return Added component by reference.
		*/
		template<typename ComponentType, typename... Args>
		requires std::is_constructible_v<ComponentType, Args...>
		ComponentType& AddComponent(Entity const entity, Args&&... args)& noexcept;

		/**
		 * @brief Get a component from the ECS.
		 * @tparam ComponentType Type of component to get.
		 * @param id id to get the component from.
		 * @return Component.
		 * @note component must be added before it is safe to call this.
		*/
		template <typename ComponentType>
		[[nodiscard]] ComponentType const& GetComponent(EntityID id) const& noexcept;
		/**
		 * @brief Get a component from the ECS.
		 * @tparam ComponentType Type of component to get.
		 * @param entity entity to get the component from.
		 * @return Component.
		 * @note component must be added before it is safe to call this.
		*/
		template <typename ComponentType>
		[[nodiscard]] ComponentType const& GetComponent(Entity const entity) const& noexcept;
		/**
		 * @brief Get a component from the ECS.
		 * @tparam ComponentType Type of component to get.
		 * @param id id to get the component from.
		 * @return Component.
		 * @note component must be added before it is safe to call this.
		*/
		template <typename ComponentType>
		[[nodiscard]] ComponentType& GetComponent(EntityID id)& noexcept;
		/**
		 * @brief Get a component from the ECS.
		 * @tparam ComponentType Type of component to get.
		 * @param entity entity to get the component from.
		 * @return Component.
		 * @note component must be added before it is safe to call this.
		*/
		template <typename ComponentType>
		[[nodiscard]] ComponentType& GetComponent(Entity const entity)& noexcept;

		/**
		 * @brief Remove a component from an entity.
		 * @tparam ComponentType Type of component to construct.
		 * @param id to remove the component from.
		 * @return If the component was removed.
		*/
		template <typename ComponentType>
		bool RemoveComponent(EntityID id)& noexcept;
		/**
		 * @brief Remove a component from an entity.
		 * @tparam ComponentType Type of component to remove.
		 * @param entity entity to remove the component from.
		 * @return If the component was removed.
		*/
		template <typename ComponentType>
		bool RemoveComponent(Entity const entity)& noexcept;

		/**
		 * @brief Check if an entity has a specific component.
		 * @tparam ComponentType Type of component to construct.
		 * @param entity entity to check.
		 * @return If the entity has the component.
		*/
		template<typename ComponentType>
		[[nodiscard]] bool HasComponent(Entity const entity) const& noexcept;
		/**
		 * @brief Check if an entity has a specific component.
		 * @tparam ComponentType Type of component to construct.
		 * @param id entity to check.
		 * @return If the entity has the component.
		*/
		template<typename ComponentType>
		[[nodiscard]] bool HasComponent(EntityID id) const& noexcept;

		// Create an entity and add it to the ECS
		[[nodiscard]] Entity CreateEntity()& noexcept;

		// Destroy an Entity & remove it from the ECS
		void DestroyEntity(Entity entity)& noexcept;
		// Destroy an Entity & remove it from the ECS
		void DestroyEntity(EntityID id)& noexcept;

		// Checks if the entity is valid
		[[nodiscard]] bool IsValid(Entity entity) const& noexcept;
		// Checks if the entity is valid
		[[nodiscard]] bool IsValid(EntityID id) const& noexcept;
	private:
		std::unique_ptr<ECSImpl> m_pImpl;
	};

}

#endif
