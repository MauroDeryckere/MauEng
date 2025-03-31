#include "GameScene.h"

#include "ServiceLocator.h"

#include <iostream>

#include "GameTime.h"

namespace MauGam
{
	void GameScene::OnLoad()
	{
		Scene::OnLoad();

		std::cout << "Demo Scene loaded! \n";
		using namespace MauRen;

		// init meshes
		Mesh m1{ "Models/Gun.obj" };
		Mesh m2{ "Models/Skull.obj" };

		MauEng::ServiceLocator::GetRenderer().UpLoadModel(m1);
		MauEng::ServiceLocator::GetRenderer().UpLoadModel(m2);

		MeshInstance mi1{ m2 };
		mi1.Translate({ 5, 20,  -3 });
		mi1.Scale({ .3f, .3f, .3f });

		MeshInstance mi2{ m2 };
		mi2.Translate({ -5, 20,  -8 });
		mi2.Scale({ .3f, .3f, .3f });

		MeshInstance mi3{ m1 };
		mi3.Translate({ 0, 0,  0 });
		mi3.Rotate(glm::radians(90.f), {1, 0,  0});
		mi3.Scale({ 5.f, 5.f, 5.f });

		m_Mehses.emplace_back(mi1);
		m_Mehses.emplace_back(mi2);
		m_Mehses.emplace_back(mi3);
	}

	void GameScene::Tick()
	{
		Scene::Tick();

		float const rotationSpeed{ glm::radians(90.0f) }; // 90 degrees per second
		m_Mehses[0].Rotate(rotationSpeed * MauEng::Time::GetInstance().ElapsedSec(), glm::vec3(0.0f, 0.0f, 1.0f));
		m_Mehses[1].Rotate(rotationSpeed * MauEng::Time::GetInstance().ElapsedSec(), glm::vec3(0.0f, 0.0f, 1.0f));
		m_Mehses[2].Rotate(rotationSpeed * MauEng::Time::GetInstance().ElapsedSec(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void GameScene::OnRender()
	{
		Scene::OnRender();

		for (auto const& m : m_Mehses)
		{
			m.Draw();
		}
	}
}
