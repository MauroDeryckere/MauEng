#include "ECSTestScene.h"

#include <execution>
#include <random>

namespace MauGam
{
	ECSTestScene::ECSTestScene()
	{
		ME_PROFILE_FUNCTION()

		using namespace MauEng;

		m_CameraManager.GetActiveCamera().SetPosition(glm::vec3{ 0.f, 2, 4 });
		m_CameraManager.GetActiveCamera().SetFOV(60.f);

		m_CameraManager.GetActiveCamera().Focus({ 0,0,1 });
		m_CameraManager.GetActiveCamera().SetFar(300);
		{
			Entity entSpider{ CreateEntity() };

			auto& transform = entSpider.GetComponent<CTransform>();
			MauCor::Rotator const rot{ 0, 45 };
			transform.Rotate(rot);
			//transform.Translate({ 0, 2,  0 });
			//transform.Scale({ 5.f, 5.f, 5.f });

			//entSpider.AddComponent<CStaticMesh>("Resources/Models/old_rusty_car/old_rusty_car.glb");
			//entSpider.AddComponent<CStaticMesh>("Resources/Models/old_rusty_car/scene.gltf");
			entSpider.AddComponent<CStaticMesh>("Resources/Models/Spider/spider.obj");
		}

		//{
		//	Entity entGUN{ CreateEntity() };

		//	auto& transform = entGUN.GetComponent<CTransform>();

		//	transform.Translate({ 0, 2,  0 });
		//	transform.Scale({ 5.f, 5.f, 5.f });

		//	entGUN.AddComponent<CStaticMesh>("Resources/Models/Gun.obj");
		//}

		//{
		//	Entity entSKULL{ CreateEntity() };
		//	auto& transform = entSKULL.GetComponent<CTransform>();

		//	transform.Translate({ 5, 5,  -20 });
		//	transform.Scale({ .3f, .3f, .3f });
		//	transform.Rotate({ 270, 0, 0 });

		//	entSKULL.AddComponent<CStaticMesh>("Resources/Models/Skull.obj");
		//}

		//bool constexpr ENABLE_HIGH_INSTANCE_TEST{ true };

		//if constexpr (ENABLE_HIGH_INSTANCE_TEST)
		//{
		//	std::random_device rd;  // Random device for seed 
		//	std::mt19937 gen(rd()); // Mersenne Twister generator
		//	std::uniform_real_distribution<float> dis(-30.0f, 30.0f); // Random translation range

		//	for (size_t i { 0 }; i < 100'000; i++)
		//	{
		//		Entity entGUN{ CreateEntity() };
		//		auto& transform = entGUN.GetComponent<CTransform>();
		//		transform.Translate({ dis(gen), dis(gen), dis(gen) });

		//		entGUN.AddComponent<CStaticMesh>("Resources/Models/Gun.obj");
		//	}
		//}


		auto& input{ INPUT_MANAGER };
		input.BindAction("MoveUp", MauEng::KeyInfo{SDLK_UP, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveLeft", MauEng::KeyInfo{ SDLK_LEFT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveRight", MauEng::KeyInfo{ SDLK_RIGHT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveDown", MauEng::KeyInfo{ SDLK_DOWN, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("RotUp", MauEng::KeyInfo{ SDLK_I, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotLeft", MauEng::KeyInfo{ SDLK_J, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotRight", MauEng::KeyInfo{ SDLK_L, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotDown", MauEng::KeyInfo{ SDLK_K, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("Sprint", MauEng::KeyInfo{ SDLK_A, MauEng::KeyInfo::ActionType::Held });


		input.BindAction("Rotate", MauEng::MouseInfo{ {},   MauEng::MouseInfo::ActionType::Moved });
	}

	void ECSTestScene::OnLoad()
	{
		ME_PROFILE_FUNCTION()

		Scene::OnLoad();
	}

	void ECSTestScene::Tick()
	{
		ME_PROFILE_FUNCTION()

		Scene::Tick();

		auto const& input{ INPUT_MANAGER };

		bool isSprinting{ false };
		auto constexpr sprintModifier{ 6.f };
		auto constexpr movementSpeed{ 20.f };

		if (input.IsActionExecuted("Sprint"))
		{
			isSprinting = true;
		}

		if (input.IsActionExecuted("MoveUp"))
		{
			m_CameraManager.GetActiveCamera().Translate({ 0.f, 0.f, movementSpeed * TIME.ElapsedSec() * (isSprinting ? sprintModifier : 1) });
		}
		if (input.IsActionExecuted("MoveDown"))
		{
			m_CameraManager.GetActiveCamera().Translate({ 0.f, 0.f, -movementSpeed * TIME.ElapsedSec() * (isSprinting ? sprintModifier : 1) });
		}
		if (input.IsActionExecuted("MoveLeft"))
		{
			m_CameraManager.GetActiveCamera().Translate({ -movementSpeed * TIME.ElapsedSec() * (isSprinting ? sprintModifier : 1), 0.f, 0.f });
		}
		if (input.IsActionExecuted("MoveRight"))
		{
			m_CameraManager.GetActiveCamera().Translate({ movementSpeed * TIME.ElapsedSec() * (isSprinting ? sprintModifier : 1), 0.f, 0.f });
		}

		float constexpr keyboardRotSpeed{ 10 };
		if (input.IsActionExecuted("RotLeft"))
		{
			float const rot{ -keyboardRotSpeed * TIME.ElapsedSec() * 3 };
			m_CameraManager.GetActiveCamera().RotateX(rot);
		}
		if (input.IsActionExecuted("RotRight"))
		{
			float const rot{ keyboardRotSpeed * TIME.ElapsedSec() * 3 };
			m_CameraManager.GetActiveCamera().RotateX(rot);
		}
		if (input.IsActionExecuted("RotUp"))
		{
			float const rot{ keyboardRotSpeed * TIME.ElapsedSec() };
			m_CameraManager.GetActiveCamera().RotateY(rot);
		}
		if (input.IsActionExecuted("RotDown"))
		{
			float const rot{ -keyboardRotSpeed * TIME.ElapsedSec() };
			m_CameraManager.GetActiveCamera().RotateY(rot);
		}

		float constexpr mouseRotSpeed{ 60 };
		if (input.IsActionExecuted("Rotate"))
		{
			auto const mouseMovement{ input.GetDeltaMouseMovement() };
			float const rot{ mouseRotSpeed * TIME.ElapsedSec() };

			m_CameraManager.GetActiveCamera().RotateX(mouseMovement.first * rot);
			m_CameraManager.GetActiveCamera().RotateY(-mouseMovement.second * rot);
		}

		using namespace MauEng;
		{
			std::random_device rd;  // Random device for seed 
			std::mt19937 gen(rd()); // Mersenne Twister generator
			std::uniform_real_distribution<float> dis(0,2); // Random translation range

			int r = dis(gen);

			float constexpr rotationSpeed{ 90.0f * 2 };
			ME_PROFILE_SCOPE("UPDATES")
			{
				//{
				//	ME_PROFILE_SCOPE("VIEW")

				//	auto view{ GetECSWorld().View<CStaticMesh, CTransform>() };
				//	view.Each([](CStaticMesh const& m, CTransform& t)
				//		{
				//			t.Rotate({ 0, rotationSpeed * TIME.ElapsedSec() });
				//		}, std::execution::par_unseq);
				//}

				{
					// Group is faster.
					//ME_PROFILE_SCOPE("GROUP")

					//MauCor::Rotator const rot{ 0, rotationSpeed * TIME.ElapsedSec() };
					//auto group{ GetECSWorld().Group<CStaticMesh, CTransform>() };
					//group.Each([&rot, &r](CStaticMesh const& m, CTransform& t)
					//	{
					//		if (r++ % 2)
					//		{
					//			t.Rotate(rot);
					//		}

					//	}, std::execution::par_unseq);
				}

			}
		}
	}
}
