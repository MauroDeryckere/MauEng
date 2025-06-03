#include "DemoScene.h"

namespace MauGam
{
	DemoScene::DemoScene()
	{
		ME_PROFILE_FUNCTION()

		using namespace MauEng;

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
			cLight.intensity = 500.f;
			cLight.direction_position = {0, -1, 0};
			cLight.castShadows = false;
			cLight.lightColour = { 1, 1, .6 };
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


		input.BindAction("Rotate", MauEng::MouseInfo{ {}, MauEng::MouseInfo::ActionType::Moved });
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
	
	}

	void DemoScene::OnRender() const
	{
		Scene::OnRender();

	}
}