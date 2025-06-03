#include "DemoScene.h"

namespace MauGam
{
	DemoScene::DemoScene()
	{
		ME_PROFILE_FUNCTION()

		using namespace MauEng;

		switch (m_Demo) {
		case EDemo::Sponza:
			{
				m_CameraManager.GetActiveCamera().SetPosition({ 0, 20, 10 });
				m_CameraManager.GetActiveCamera().SetFOV(60.f);

				m_CameraManager.GetActiveCamera().Focus({ 1,1, 3 });
				m_CameraManager.GetActiveCamera().SetFar(1000);

				{
					Entity enttSponza{ CreateEntity() };

					auto& transform{ enttSponza.GetComponent<CTransform>() };
					float constexpr SCALE{ 10.f };
					transform.Scale({ SCALE, SCALE, SCALE });

					enttSponza.AddComponent<CStaticMesh>("Resources/Models/Sponza/glTF/Sponza.gltf");
				}

				{
					Entity enttDirLight{ CreateEntity() };
					auto& cLight{ enttDirLight.AddComponent<CLight>() };
					cLight.intensity = 2000.f;
					cLight.direction_position = { 0, -1, 0 };
					cLight.castShadows = false;
					cLight.lightColour = { 1, 1, .9 };
				}

			}
			break;
		case EDemo::Chess:
			{
				m_CameraManager.GetActiveCamera().SetPosition({ 0, 20, 10 });
				m_CameraManager.GetActiveCamera().SetFOV(60.f);

				m_CameraManager.GetActiveCamera().Focus({ 1,1, 3 });
				m_CameraManager.GetActiveCamera().SetFar(1000);

				SetSceneAABBOverride({ -100, -100, -100 }, { 100, 100, 100 });

				{
					Entity enttGame{ CreateEntity() };

					auto& transform{ enttGame.GetComponent<CTransform>() };
					transform.Scale({ 100, 100, 100 });

					enttGame.AddComponent<CStaticMesh>("Resources/Models/ABeautifulGame/GLTF/ABeautifulGame.gltf");
				}

				{
					Entity enttDirLight{ CreateEntity() };
					auto& cLight{ enttDirLight.AddComponent<CLight>() };
					cLight.intensity = 2000.f;
					cLight.direction_position = { -1, -1, -.5 };
					cLight.castShadows = false;
					cLight.lightColour = { 1, 1, .9 };
				}
			}
			break;
		case EDemo::COUNT:
			break;
		}

		SetupInput();
	}

	void DemoScene::OnLoad()
	{
		ME_PROFILE_FUNCTION()

		Scene::OnLoad();

		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "Demo Scene Loaded! ");
	}

	void DemoScene::Tick()
	{
		Scene::Tick();

		HandleInput();
	}

	void DemoScene::OnRender() const
	{
		Scene::OnRender();
	}

	void DemoScene::SetupInput()
	{
		auto& input{ INPUT_MANAGER };
		input.BindAction("MoveUp", MauEng::KeyInfo{ SDLK_UP, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveLeft", MauEng::KeyInfo{ SDLK_LEFT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveRight", MauEng::KeyInfo{ SDLK_RIGHT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveDown", MauEng::KeyInfo{ SDLK_DOWN, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("Sprint", MauEng::KeyInfo{ SDLK_A, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("Rotate", MauEng::MouseInfo{ {}, MauEng::MouseInfo::ActionType::Moved });


		input.BindAction("ToggleShadows", MauEng::KeyInfo{ SDLK_F4, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("DownLightIntensity", MauEng::KeyInfo{ SDLK_F5, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("UpLightIntensity", MauEng::KeyInfo{ SDLK_F6, MauEng::KeyInfo::ActionType::Up });
	}

	void DemoScene::HandleInput()
	{
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

		float constexpr mouseRotSpeed{ 60 };
		if (input.IsActionExecuted("Rotate"))
		{
			auto const mouseMovement{ input.GetDeltaMouseMovement() };
			float const rot{ mouseRotSpeed * TIME.ElapsedSec() };

			m_CameraManager.GetActiveCamera().RotateX(mouseMovement.first * rot);
			m_CameraManager.GetActiveCamera().RotateY(-mouseMovement.second * rot);
		}

		float constexpr LIGHT_ADJUSTMENT{ 200.f };
		if (input.IsActionExecuted("DownLightIntensity"))
		{
			auto view{ GetECSWorld().View<MauEng::CLight>() };
			view.Each([LIGHT_ADJUSTMENT](MauEng::CLight& light)
				{
					light.intensity -= LIGHT_ADJUSTMENT;
				});
		}

		if (input.IsActionExecuted("UpLightIntensity"))
		{
			auto view{ GetECSWorld().View<MauEng::CLight>() };
			view.Each([LIGHT_ADJUSTMENT](MauEng::CLight& light)
				{
					light.intensity += LIGHT_ADJUSTMENT;
				});
		}

		if (input.IsActionExecuted("ToggleShadows"))
		{
			auto view{ GetECSWorld().View<MauEng::CLight>() };
			view.Each([](MauEng::CLight& light)
				{
					light.castShadows = !light.castShadows;
				});
		}
	}
}
