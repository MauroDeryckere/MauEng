#ifndef MAUENG_SCENEMANAGER_H
#define MAUENG_SCENEMANAGER_H

#include "Singleton.h"
#include "Scene.h"

#include <memory>

namespace MauEng
{
	class CameraManager;

	class SceneManager final : public MauCor::Singleton<SceneManager>
	{
	public:
		void LoadScene(std::unique_ptr<Scene> pScene);

		void FixedUpdate();
		void Render(glm::vec2 const& screenSize) const;
		void Tick();

		void UpdateCamerasAspectRatio(float aspectRatio) noexcept;

		SceneManager(SceneManager const&) = delete;
		SceneManager(SceneManager&&) = delete;
		SceneManager& operator=(SceneManager const&) = delete;
		SceneManager& operator=(SceneManager&&) = delete;

	private:
		friend class Singleton<SceneManager>;
		SceneManager() = default;
		virtual ~SceneManager() override;

		// Only support one scene for now
		std::unique_ptr<Scene> m_Scene;
	};
}

#endif