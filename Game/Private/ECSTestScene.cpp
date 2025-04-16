#include "ECSTestScene.h"

namespace MauGam
{
	ECSTestScene::ECSTestScene()
	{
		MauEng::Entity ent{ CreateEntity() };
	}

	void ECSTestScene::OnLoad()
	{
		Scene::OnLoad();
	}

	void ECSTestScene::Tick()
	{
		Scene::Tick();
	}

	void ECSTestScene::OnRender() const
	{
		Scene::OnRender();

	}
}
