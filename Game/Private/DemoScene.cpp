#include "DemoScene.h"

namespace MauGam
{
	DemoScene::DemoScene()
	{
		ME_PROFILE_FUNCTION()

		using namespace MauEng;

		switch (m_Demo)
		{
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
				m_CameraManager.GetActiveCamera().SetPosition({ -75, 65, -75 });
				m_CameraManager.GetActiveCamera().SetFOV(60.f);

				m_CameraManager.GetActiveCamera().Focus({ 0,10, 0 });
				m_CameraManager.GetActiveCamera().SetFar(500);

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

				{
					Entity enttPLight{ CreateEntity() };
					auto& cLight{ enttPLight.AddComponent<CLight>() };
					cLight.type = ELightType::POINT;
					cLight.intensity = 100'000.f;
					cLight.direction_position = { 10, 40, 10 };
					cLight.lightColour = { 1, 0, 0 };
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

		bool const shouldSceneRotate{ m_Demo == EDemo::Chess };
		if (m_Rotate and shouldSceneRotate)
		{
			using namespace MauEng;
			float constexpr ROTATION_SPEED{ 15.f };
			MauCor::Rotator const rot{ 0, ROTATION_SPEED * TIME.ElapsedSec() };
			auto group{ GetECSWorld().Group<CStaticMesh, CTransform>() };
			group.Each([&rot](CStaticMesh const& m, CTransform& t)
				{
					t.Rotate(rot);
				}, std::execution::par_unseq);
		}

	}

	void DemoScene::OnRender() const
	{
		Scene::OnRender();

		if (m_DebugRenderLight)
		{
			auto view{ GetECSWorld().View<MauEng::CLight>() };
			view.Each([](MauEng::CLight const& l)
				{
					switch (l.type)
					{
					case MauEng::ELightType::DIRECTIONAL:
					{
						glm::vec3 constexpr start{ 0, 100, 0 };
						glm::vec3 const dir{ glm::normalize(l.direction_position) };
						float const length{ std::clamp(l.intensity / 10000.f, 5.f, 40.f) };
						glm::vec3 const end{ start + dir * length };

						DEBUG_RENDERER.DrawArrow(start, end, {}, l.lightColour, 1.f);

						break;
					}

					case MauEng::ELightType::POINT:
						DEBUG_RENDERER.DrawSphere(l.direction_position, std::clamp(l.intensity / 10000.f, 2.f, 20.f), {}, l.lightColour);
						break;
					default:
						break;
					}
				});
		}
	}

	void DemoScene::SetupInput()
	{
		auto& input{ INPUT_MANAGER };
		input.BindAction("MoveUp", MauEng::KeyInfo{ SDLK_UP, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveLeft", MauEng::KeyInfo{ SDLK_LEFT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveRight", MauEng::KeyInfo{ SDLK_RIGHT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveDown", MauEng::KeyInfo{ SDLK_DOWN, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveUp", MauEng::KeyInfo{ SDLK_W, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveLeft", MauEng::KeyInfo{ SDLK_A, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveRight", MauEng::KeyInfo{ SDLK_D, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveDown", MauEng::KeyInfo{ SDLK_S, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("Sprint", MauEng::KeyInfo{ SDLK_LCTRL, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("Rotate", MauEng::MouseInfo{ {}, MauEng::MouseInfo::ActionType::Moved });


		input.BindAction("ToggleLightDebugRendering", MauEng::KeyInfo{ SDLK_F2, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleLights", MauEng::KeyInfo{ SDLK_F3, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleShadows", MauEng::KeyInfo{ SDLK_F4, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("DownLightIntensity", MauEng::KeyInfo{ SDLK_F5, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("UpLightIntensity", MauEng::KeyInfo{ SDLK_F6, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleRotation", MauEng::KeyInfo{ SDLK_F7, MauEng::KeyInfo::ActionType::Up });
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
					ME_LOG_INFO(MauCor::LogCategory::Game, "Light intensity: {}", light.intensity);
				});
		}

		if (input.IsActionExecuted("UpLightIntensity"))
		{
			auto view{ GetECSWorld().View<MauEng::CLight>() };
			view.Each([LIGHT_ADJUSTMENT](MauEng::CLight& light)
				{
					light.intensity += LIGHT_ADJUSTMENT;
					ME_LOG_INFO(MauCor::LogCategory::Game, "Light intensity: {}", light.intensity);
				});
		}

		if (input.IsActionExecuted("ToggleShadows"))
		{
			m_CastShadows = not m_CastShadows;
			ME_LOG_INFO(MauCor::LogCategory::Game, "Shadows: {}", m_CastShadows);

			auto view{ GetECSWorld().View<MauEng::CLight>() };
			view.Each([this](MauEng::CLight& light)
				{
					light.castShadows = m_CastShadows;
				});
		}
		
		if (input.IsActionExecuted("ToggleLights"))
		{
			uint8_t currModeID{ static_cast<uint8_t>(m_LightMode) };
			++currModeID;
			currModeID %= static_cast<uint8_t>(ELightMode::COUNT);
			m_LightMode = static_cast<ELightMode>(currModeID);

			std::string lightMode{"NONE"};
			switch (m_LightMode) {
			case ELightMode::PointOnly:
				lightMode = "Point Only";
				break;
			case ELightMode::DirOnly:
				lightMode = "Directional Only";
				break;
			case ELightMode::PointAndDir:
				lightMode = "Point And Directional";
				break;
			}
			ME_LOG_INFO(MauCor::LogCategory::Game, "Light mode: {}", lightMode);

			auto view{ GetECSWorld().View<MauEng::CLight>() };
			view.Each([this](MauEng::CLight& light)
				{
					switch (m_LightMode) {
					case ELightMode::PointOnly:
						if (MauEng::ELightType::POINT == light.type)
						{
							light.isEnabled = true;
						}
						else
						{
							light.isEnabled = false;
						}
						break;

					case ELightMode::DirOnly:
						if (MauEng::ELightType::DIRECTIONAL == light.type)
						{
							light.isEnabled = true;
						}
						else
						{
							light.isEnabled = false;
						}
						break;
					case ELightMode::PointAndDir:
						if (MauEng::ELightType::DIRECTIONAL == light.type or MauEng::ELightType::POINT == light.type)
						{
							light.isEnabled = true;
						}
						break;

					case ELightMode::COUNT:
						break;
					}
				});
		}

		if (input.IsActionExecuted("ToggleLightDebugRendering"))
		{
			m_DebugRenderLight = not m_DebugRenderLight;
			ME_LOG_INFO(MauCor::LogCategory::Game, "Debug render light: {}", m_DebugRenderLight);
		}

		if (input.IsActionExecuted("ToggleRotation"))
		{
			m_Rotate = not m_Rotate;
			ME_LOG_INFO(MauCor::LogCategory::Game, "Scene rotation: {}", m_Rotate);
		}
	}

}
