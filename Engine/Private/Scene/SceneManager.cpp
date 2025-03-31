#include "Scene/SceneManager.h"

#include "ServiceLocator.h"

#include "glm/glm.hpp"

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

	void SceneManager::Render(glm::mat4 const& view, glm::mat4 const& proj)
	{
		m_Scene->OnRender();

		ServiceLocator::GetRenderer().Render(view, proj);
	}

	void SceneManager::Tick()
	{
		m_Scene->Tick();
	}

	SceneManager::~SceneManager()
	{
		m_Scene->OnUnload();
	}
}
