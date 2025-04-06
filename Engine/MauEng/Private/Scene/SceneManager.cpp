#include "Scene/SceneManager.h"

#include "ServiceLocator.h"

namespace MauEng
{
	void SceneManager::LoadScene(std::unique_ptr<Scene> pScene)
	{
		m_Scene = std::move(pScene);
		m_Scene->OnLoad();
	}

	void SceneManager::FixedUpdate()
	{
		//TODO
	}

	void SceneManager::Render() const
	{
		m_Scene->OnRender();

		Renderer().Render(m_Scene->GetCameraManager().GetActiveCamera().GetViewMatrix(), m_Scene->GetCameraManager().GetActiveCamera().GetProjectionMatrix());
	}

	void SceneManager::Tick()
	{
		m_Scene->Tick();
	}

	void SceneManager::UpdateCamerasAspectRatio(float aspectRatio) noexcept
	{
		m_Scene->GetCameraManager().GetActiveCamera().SetAspectRatio(aspectRatio);
	}

	SceneManager::~SceneManager()
	{
		m_Scene->OnUnload();
	}
}
