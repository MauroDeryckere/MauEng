#include "ECSTestScene.h"

namespace MauGam
{
	ECSTestScene::ECSTestScene()
	{
		using namespace MauEng;

		ME_PROFILE_FUNCTION()

		m_CameraManager.GetActiveCamera().SetPosition(glm::vec3{ 0.f, 2, 4 });
		m_CameraManager.GetActiveCamera().SetFOV(60.f);

		m_CameraManager.GetActiveCamera().Focus({ 0,0,1 });

		Entity ent{ CreateEntity() };

		auto& transform = ent.GetComponent<CTransform>();

		transform.Translate({ 0, 2,  0 });
		transform.Scale({ 5.f, 5.f, 5.f });

		auto& mesh = ent.AddComponent<CStaticMesh>("Resources/Models/Gun.obj");
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
