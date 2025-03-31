#include "Scene/SceneManager.h"

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

	void SceneManager::Render()
	{
		m_Scene->OnRender();
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
