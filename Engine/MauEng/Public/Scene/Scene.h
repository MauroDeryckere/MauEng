#ifndef MAUENG_SCENE_H
#define MAUENG_SCENE_H

#include "CameraManager.h"

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

		[[nodiscard]] CameraManager const& GetCameraManager() const noexcept { return m_CameraManager; }
		[[nodiscard]] CameraManager& GetCameraManager() noexcept { return m_CameraManager; }

		Scene(Scene const&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene const&) = delete;
		Scene& operator=(Scene&&) = delete;

	protected:
		CameraManager m_CameraManager{ };

	private:

	};
}

#endif