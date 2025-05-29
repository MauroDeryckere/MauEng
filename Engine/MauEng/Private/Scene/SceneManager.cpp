#include "Scene/SceneManager.h"

#include "InternalServiceLocator.h"
#include "ServiceLocator.h"

namespace MauEng
{
	void SceneManager::LoadScene(std::unique_ptr<Scene> pScene)
	{
		ME_PROFILE_FUNCTION()

		m_Scene = std::move(pScene);
		m_Scene->OnLoad();
	}

	void SceneManager::FixedUpdate()
	{
		ME_PROFILE_FUNCTION()
		//TODO
	}

	void SceneManager::Render(glm::vec2 const& screenSize) const
	{
		ME_PROFILE_FUNCTION()

		m_Scene->OnRender();

		RENDERER.Render(m_Scene->GetCameraManager().GetActiveCamera().GetViewMatrix(), 
						m_Scene->GetCameraManager().GetActiveCamera().GetProjectionMatrix(), 
						screenSize
						);
	}

	void SceneManager::Tick()
	{
		ME_PROFILE_FUNCTION()

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
