#include "ECSTestScene.h"

namespace MauGam
{
	ECSTestScene::ECSTestScene()
	{
		MauEng::Entity ent{ CreateEntity() };

		auto& transform = ent.GetComponent<MauEng::CTransform>();

	}

	void ECSTestScene::OnLoad()
	{
		Scene::OnLoad();
	}

	void ECSTestScene::Tick()
	{
		Scene::Tick();

		GetECSWorld().ForEach<MauEng::CTransform>(
			[] (MauEng::ECS::EntityID id, MauEng::CTransform& t)
			{
				std::cout << id << " " << t.position.x << "\n";
			});
	}

	void ECSTestScene::OnRender() const
	{
		Scene::OnRender();

	}
}
