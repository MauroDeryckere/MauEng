#ifndef MAUENG_SCENE_H
#define MAUENG_SCENE_H

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
		virtual void Tick(){}

		// Called to render the scene
		virtual void OnRender(){}

		// Called when the scene is unloaded
		virtual void OnUnload(){}

		Scene(Scene const&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene const&) = delete;
		Scene& operator=(Scene&&) = delete;
	protected:

	private:

	};
}

#endif