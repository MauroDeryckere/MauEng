#ifndef MAUENG_SCENE_H
#define MAUENG_SCENE_H

#include "CameraManager.h"
//TODO fix
//#include "ECSWorld.h"

#include "ServiceLocator.h"
#include "../../ECS/Public/ECSWorld.h"
#include "Entity.h"

#include "Components/CTransform.h"

#include "Timer/TimerManager.h"

namespace MauEng
{
	// Base scene class to inherit from when creating a scene for the game
	class Scene
	{
	public:
		Scene();
		virtual ~Scene() = default;

		// Called when the scene is loaded
		virtual void OnLoad(){}

		// Called each frame
		virtual void Tick();

		// Called to render the scene
		virtual void OnRender() const;

		// Called when the scene is unloaded
		virtual void OnUnload(){}

		void SetSceneAABBOverride(glm::vec3 const& min, glm::vec3 const& max);

#pragma region ECS
		[[nodiscard]] Entity CreateEntity(glm::vec3 const& pos = {});
		void DestroyEntity(Entity entity);

		[[nodiscard]] ECS::ECSWorld& GetECSWorld() noexcept { return m_ECSWorld; }
		[[nodiscard]] ECS::ECSWorld const& GetECSWorld() const noexcept { return m_ECSWorld; }
#pragma endregion

		[[nodiscard]] CameraManager const& GetCameraManager() const noexcept { return m_CameraManager; }
		[[nodiscard]] CameraManager& GetCameraManager() noexcept { return m_CameraManager; }

		[[nodiscard]] MauCor::TimerManager const& GetTimerManager() const noexcept { return m_TimerManager; }
		[[nodiscard]] MauCor::TimerManager& GetTimerManager() noexcept { return m_TimerManager; }

		Scene(Scene const&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene const&) = delete;
		Scene& operator=(Scene&&) = delete;

	protected:
		CameraManager m_CameraManager{ };
		MauCor::TimerManager m_TimerManager{};

	private:
		mutable ECS::ECSWorld m_ECSWorld{ };

	};
}

#endif