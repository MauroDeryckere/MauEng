#ifndef MAUENG_SCENE_H
#define MAUENG_SCENE_H

#include "CameraManager.h"
//TODO fix
//#include "ECSWorld.h"

#include "../../ECS/Public/ECSWorld.h"
#include "../../ECS/Public/Entity.h"

#include "Components/CTransform.h"

namespace MauEng
{
	// Base scene class to inherit from when creating a scene for the game
	class Scene
	{
	public:
		Scene() = default;
		virtual ~Scene() = default;

		// Called when the scene is loaded
		virtual void OnLoad(){}

		// Called each frame
		virtual void Tick()
		{
			m_CameraManager.Tick();
		}

		// Called to render the scene
		virtual void OnRender() const {}

		// Called when the scene is unloaded
		virtual void OnUnload(){}

#pragma region ECS
		[[nodiscard]] Entity CreateEntity();
		void DestroyEntity(Entity entity);

		[[nodiscard]] ECS::ECSWorld& GetECSWorld() noexcept { return m_ECSWorld; }
		[[nodiscard]] ECS::ECSWorld const& GetECSWorld() const noexcept { return m_ECSWorld; }
#pragma endregion

		[[nodiscard]] CameraManager const& GetCameraManager() const noexcept { return m_CameraManager; }
		[[nodiscard]] CameraManager& GetCameraManager() noexcept { return m_CameraManager; }

		Scene(Scene const&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene const&) = delete;
		Scene& operator=(Scene&&) = delete;

	protected:
		CameraManager m_CameraManager{ };

	private:
		ECS::ECSWorld m_ECSWorld{ };

	};
}

#endif