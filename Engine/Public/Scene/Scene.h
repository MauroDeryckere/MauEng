#ifndef MAUENG_SCENE_H
#define MAUENG_SCENE_H

#include "Camera.h"

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
			m_Camera.Update();
		}

		// Called to render the scene
		virtual void OnRender(){}

		// Called when the scene is unloaded
		virtual void OnUnload(){}

		[[nodiscard]] Camera const& GetCamera() const noexcept { return m_Camera; }

		Scene(Scene const&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene const&) = delete;
		Scene& operator=(Scene&&) = delete;

	protected:
		Camera m_Camera{ };

	private:

	};
}

#endif