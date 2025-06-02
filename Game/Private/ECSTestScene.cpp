#include "ECSTestScene.h"

#include <execution>
#include <random>

namespace MauGam
{
	ECSTestScene::ECSTestScene()
	{
		ME_PROFILE_FUNCTION()

			using namespace MauEng;

		m_CameraManager.GetActiveCamera().SetPosition(glm::vec3{ -200.f, 40, -100 });
		m_CameraManager.GetActiveCamera().SetFOV(60.f);

		m_CameraManager.GetActiveCamera().Focus({ 0,0,1 });
		m_CameraManager.GetActiveCamera().SetFar(1000);
		{
			Entity enttCar{ CreateEntity() };

			auto& transform{ enttCar.GetComponent<CTransform>() };
			transform.Scale({ .5f, .5f, .5f });

			//enttCar.AddComponent<CStaticMesh>("Resources/Models/old_rusty_car/scene.gltf");
		}
		{
			Entity enttGame{ CreateEntity() };

			auto& transform{ enttGame.GetComponent<CTransform>() };
			transform.Scale({ 100, 100, 100 });

			//enttGame.AddComponent<CStaticMesh>("Resources/Models/ABeautifulGame/GLTF/ABeautifulGame.gltf");
		}

		
		{
			Entity enttHelmet{ CreateEntity() };

			auto& transform{ enttHelmet.GetComponent<CTransform>() };
			transform.Scale({ 100.f, 100.f, 100.f });
			enttHelmet.AddComponent<CStaticMesh>("Resources/Models/FlightHelmet/glTF/FlightHelmet.gltf");
		}

		{
			//TODO does not work
			Entity enttSponza{ CreateEntity() };

			auto& transform{ enttSponza.GetComponent<CTransform>() };
			//	MauCor::Rotator const rot{ 90, 0, 180 };
				//transform.Rotate(rot);
			//	transform.Scale({ .5f, .5f, .5f });

				//enttSponza.AddComponent<CStaticMesh>("Resources/Models/Sponza/glTF/Sponza.gltf");
		}

		{
			// Note: spider has no normal map
			Entity entSpider{ CreateEntity() };
			auto& transform{ entSpider.GetComponent<CTransform>() };
			//transform.Scale({ .05f, .05f, .05f });
			//entSpider.AddComponent<CStaticMesh>("Resources/Models/Spider/spider.obj");
		}


		bool constexpr ENABLE_HIGH_INSTANCE_TEST{ false };
		uint32_t constexpr NUM_INSTANCES{ 10'000 };
		if constexpr (ENABLE_HIGH_INSTANCE_TEST)
		{
			std::random_device rd;  // Random device for seed 
			std::mt19937 gen(rd()); // Mersenne Twister generator
			std::uniform_real_distribution<float> dis(-300.0f, 300); // Random translation range

			for (size_t i{ 0 }; i < NUM_INSTANCES; i++)
			{
				// Note: spider has no normal map (scaling is a little off)
				Entity entSpider{ CreateEntity() };
				auto& transform{ entSpider.GetComponent<CTransform>() };
				transform.Translate({ dis(gen), dis(gen), dis(gen) });
				transform.Scale({ .05f, .05f, .05f });
				entSpider.AddComponent<CStaticMesh>("Resources/Models/Spider/spider.obj");
			}
		}


		auto& input{ INPUT_MANAGER };
		input.BindAction("MoveUp", MauEng::KeyInfo{ SDLK_UP, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveLeft", MauEng::KeyInfo{ SDLK_LEFT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveRight", MauEng::KeyInfo{ SDLK_RIGHT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveDown", MauEng::KeyInfo{ SDLK_DOWN, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("RotUp", MauEng::KeyInfo{ SDLK_I, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotLeft", MauEng::KeyInfo{ SDLK_J, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotRight", MauEng::KeyInfo{ SDLK_L, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotDown", MauEng::KeyInfo{ SDLK_K, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("Sprint", MauEng::KeyInfo{ SDLK_A, MauEng::KeyInfo::ActionType::Held });


		input.BindAction("Rotate", MauEng::MouseInfo{ {},   MauEng::MouseInfo::ActionType::Moved });

		//{
		//	Entity entLightTest{ CreateEntity() };

		//	auto& l = entLightTest.AddComponent<CLight>();
		//	l.type = ELightType::POINT;
		//	l.direction_position = { 0, 200, 0 };
		//	l.intensity = 1000000.f;
		//	l.lightColour = { 1.f, 1.f, 1.f };
		//}
		//{
		//	Entity entLightTest{ CreateEntity() };

		//	auto& l = entLightTest.AddComponent<CLight>();
		//	l.type = ELightType::POINT;
		//	l.direction_position = { -100.f, 50, 0 };
		//	l.intensity = 1000000.f;
		//	l.lightColour = { 1.f, 0.f, 0.f };
		//}
		{
			Entity entLightTest{ CreateEntity() };

			auto& l = entLightTest.AddComponent<CLight>();
			l.intensity = 5000.f;
			l.lightColour = { 1.f, 1.f, 1.f };
			l.direction_position = { -1, -1, 0 };
		}
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

		auto view = GetECSWorld().View<MauEng::CLight>();
		view.Each([](MauEng::CLight const& l)
		{
			switch (l.type)
			{
			case MauEng::ELightType::DIRECTIONAL:
				{
					glm::vec3 start = { 0, 100, 0 };
					glm::vec3 dir = glm::normalize(l.direction_position);
					float length = std::clamp(l.intensity / 10000.f, 2.f, 20.f);
					glm::vec3 end = start + dir * length;

					DEBUG_RENDERER.DrawArrow(start, end, {}, l.lightColour, 1.f);

					break;
				}

			case MauEng::ELightType::POINT:
				DEBUG_RENDERER.DrawSphere(l.direction_position, std::clamp(l.intensity/10000.f, 2.f, 20.f), {}, l.lightColour);
				break;
			case MauEng::ELightType::COUNT:
				break;
			default: ;
			}
		});

		//DEBUG_RENDERER.DrawCylinder({}, { 100, 100, 100 }, {}, {1,1,1}, 100);

		using namespace MauEng;
		{
			std::random_device rd;  // Random device for seed 
			std::mt19937 gen(rd()); // Mersenne Twister generator
			std::uniform_real_distribution<float> dis(0,2); // Random translation range

			//int r = dis(gen);
			int r = 1;
			float constexpr rotationSpeed{ 30.f * 2 };
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
					ME_PROFILE_SCOPE("GROUP")

					MauCor::Rotator const rot{ 0, rotationSpeed * TIME.ElapsedSec() };
					auto group{ GetECSWorld().Group<CStaticMesh, CTransform>() };
					group.Each([&rot, &r](CStaticMesh const& m, CTransform& t)
						{
							if (r++ % 2)
							{
							//	t.Rotate(rot);
							}

						}, std::execution::par_unseq);
				}

			}
		}
	}
}
