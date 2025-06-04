#include "DemoScene.h"

#include <random>

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
		case EDemo::FlightHelmet:
			{
				m_CameraManager.GetActiveCamera().SetPosition({ 0, 50, -100 });
				m_CameraManager.GetActiveCamera().SetFOV(60.f);

				m_CameraManager.GetActiveCamera().Focus({ 1,1, 3 });
				m_CameraManager.GetActiveCamera().SetFar(1000);

				m_CamSettings = ECamSettings::INDOOR;
				m_CameraManager.GetActiveCamera().SetCamSettingsIndoor();

				{
					Entity enttHelmet{ CreateEntity() };

					auto& transform{ enttHelmet.GetComponent<CTransform>() };
					transform.Scale({ 100.f, 100.f, 100.f });
					enttHelmet.AddComponent<CStaticMesh>("Resources/Models/FlightHelmet/glTF/FlightHelmet.gltf");
				}

				{
					Entity enttDirLight{ CreateEntity() };
					auto& cLight{ enttDirLight.AddComponent<CLight>() };
					cLight.intensity = 20.f;
					cLight.direction_position = { 0, -1, 0 };
					cLight.castShadows = false;
					cLight.lightColour = { 1, 1, 1 };
				}

				{
					Entity enttPLight{ CreateEntity() };
					auto& cLight{ enttPLight.AddComponent<CLight>() };
					cLight.type = ELightType::POINT;
					cLight.intensity = 100'000.f;
					cLight.direction_position = { 50.f, 50, -20 };
					cLight.lightColour = { 1, 0, 0 };
				}

				{
					Entity enttPLight{ CreateEntity() };
					auto& cLight{ enttPLight.AddComponent<CLight>() };
					cLight.type = ELightType::POINT;
					cLight.intensity = 100'000.f;
					cLight.direction_position = { -50.f, 50, -20 };
					cLight.lightColour = { 0, 0, 1 };
				}
				SetSceneAABBOverride({ -100, -100, -100 }, { 100, 100, 100 });
			}
			break;
		case EDemo::InstanceTest:
			{
				m_CameraManager.GetActiveCamera().SetPosition({ -50, 50, -50 });
				m_CameraManager.GetActiveCamera().SetFOV(60.f);

				m_CameraManager.GetActiveCamera().Focus({ 0, 0, 0 });
				m_CameraManager.GetActiveCamera().SetFar(1000);

				uint32_t constexpr NUM_INSTANCES{ 100'000 };
				// Random device for seed 
				std::random_device rd;
				// Mersenne Twister generator
				std::mt19937 gen(rd());
				// Random translation range
				std::uniform_real_distribution<float> dis(-300.0f, 300);

				float constexpr FISH_SCALE_MIN{ 10.f };
				float constexpr FISH_SCALE_MAX{ 20.f };
				std::uniform_real_distribution<float> disScale(FISH_SCALE_MIN, FISH_SCALE_MAX);

				for (size_t i{ 0 }; i < NUM_INSTANCES; i++)
				{
					float const fishScale{ disScale(gen)};
					Entity entFish{ CreateEntity() };
					auto& transform{ entFish.GetComponent<CTransform>() };
					transform.Translate({ dis(gen), dis(gen), dis(gen) });
					transform.Scale({ fishScale, fishScale, fishScale });
					entFish.AddComponent<CStaticMesh>("Resources/Models/BarramundiFish/glTF/BarramundiFish.gltf");
				}

				{
					Entity enttDirLight{ CreateEntity() };
					auto& cLight{ enttDirLight.AddComponent<CLight>() };
					cLight.intensity = 20'000.f;
					cLight.direction_position = { -1, -1, -1 };
					cLight.castShadows = false;
					cLight.lightColour = { 0.2f, 0.2f, 1 };
				}

			}
			break;
		case EDemo::DebugRendering:
			{
				m_CameraManager.GetActiveCamera().SetPosition({ -100, 100, -100 });
				m_CameraManager.GetActiveCamera().SetFOV(60.f);

				m_CameraManager.GetActiveCamera().Focus({ 1,1, 3 });
				m_CameraManager.GetActiveCamera().SetFar(800);


				{
					Entity enttDirLight{ CreateEntity() };
					auto& cLight{ enttDirLight.AddComponent<CLight>() };
					cLight.intensity = 500.f;
					cLight.direction_position = { -1, -1, .1f };
					cLight.castShadows = false;
					cLight.lightColour = { .7, .7, 1 };
				}

				SetSceneAABBOverride({ -300, -300, -300 }, { 300, 300, 300 });
			}
			break;
		}

		SetupInput();
	}

	void DemoScene::OnLoad()
	{
		ME_PROFILE_FUNCTION()

		Scene::OnLoad();
		OutputKeybinds();
	}

	void DemoScene::Tick()
	{
		ME_PROFILE_FUNCTION()

		Scene::Tick();

		HandleInput();

		bool const shouldSceneRotate{	m_Demo == EDemo::Chess or
										m_Demo == EDemo::FlightHelmet or
										m_Demo == EDemo::InstanceTest or
										m_Demo == EDemo::DebugRendering};

		if (m_Rotate and shouldSceneRotate)
		{
			if (m_Demo == EDemo::InstanceTest)
			{
				using namespace MauEng;
				float constexpr ROTATION_SPEED{ 90.f };
				MauCor::Rotator const rot{ 0, ROTATION_SPEED * TIME.ElapsedSec() };
				auto group{ GetECSWorld().Group<CStaticMesh, CTransform>() };
				group.Each([&rot](CStaticMesh const& m, CTransform& t)
					{
						t.Rotate(rot);
					}, std::execution::par_unseq);
			}
			else
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

	}

	void DemoScene::OnRender() const
	{
		ME_PROFILE_FUNCTION()

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

		if (m_Demo == EDemo::DebugRendering)
		{
			RenderDebugDemo();
		}
	}

	void DemoScene::SetupInput()
	{
		ME_PROFILE_FUNCTION()

		auto& input{ INPUT_MANAGER };
		input.BindAction("PrintInfo", MauEng::KeyInfo{ SDLK_SPACE, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleLightDebugRendering", MauEng::KeyInfo{ SDLK_F2, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleLights", MauEng::KeyInfo{ SDLK_F3, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleShadows", MauEng::KeyInfo{ SDLK_F4, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("DownLightIntensity", MauEng::KeyInfo{ SDLK_F5, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("UpLightIntensity", MauEng::KeyInfo{ SDLK_F6, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleRotation", MauEng::KeyInfo{ SDLK_F7, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleDebugRenderMode", MauEng::KeyInfo{ SDLK_F8, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("RandomizeLightColours", MauEng::KeyInfo{ SDLK_F9, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleCamSettings", MauEng::KeyInfo{ SDLK_F10, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleToneMap", MauEng::KeyInfo{ SDLK_F11, MauEng::KeyInfo::ActionType::Up });

		input.BindAction("LowerCustomExposure", MauEng::KeyInfo{ SDLK_E, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("HigherCustomExposure", MauEng::KeyInfo{ SDLK_R, MauEng::KeyInfo::ActionType::Up });


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
	}

	void DemoScene::HandleInput()
	{
		ME_PROFILE_FUNCTION()

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
					std::string lightTypeStr{ "NONE" };
					switch (light.type) {
					case MauEng::ELightType::DIRECTIONAL:
						lightTypeStr = "Directional Light " + std::to_string(light.lightID);
						break;
					case MauEng::ELightType::POINT:
						lightTypeStr = "Point Light " + std::to_string(light.lightID);
						break;

					}
					light.intensity += LIGHT_ADJUSTMENT;
					ME_LOG_INFO(MauCor::LogCategory::Game, "Light intensity ({}): {}", lightTypeStr, light.intensity);
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

		if (input.IsActionExecuted("ToggleDebugRenderMode"))
		{
			uint8_t currModeID{ static_cast<uint8_t>(m_DebugRenderMode) };
			++currModeID;
			currModeID %= static_cast<uint8_t>(EDebugRenderMode::COUNT);
			m_DebugRenderMode = static_cast<EDebugRenderMode>(currModeID);

			std::string debModeStr{ "NONE" };
			switch (m_DebugRenderMode) {
			case EDebugRenderMode::DRAW_LINES:
				debModeStr = "Draw Lines";
				break;
			case EDebugRenderMode::DRAW_RECTS:
				debModeStr = "Draw Rects";
				break;
			case EDebugRenderMode::DRAW_TRIANGLES:
				debModeStr = "Draw Triangles";
				break;
			case EDebugRenderMode::DRAW_ARROWS:
				debModeStr = "Draw Arrows";
				break;
			case EDebugRenderMode::DRAW_CIRCLES:
				debModeStr = "Draw Circles";
				break;
			case EDebugRenderMode::DRAW_SPHERES:
				debModeStr = "Draw Spheres";
				break;
			case EDebugRenderMode::DRAW_CYL:
				debModeStr = "Draw Cyls";
				break;
			case EDebugRenderMode::DRAW_POLY:
				debModeStr = "Draw Poly";
				break;
			case EDebugRenderMode::ALL:
				debModeStr = "Draw All";
				break;
			}

			ME_LOG_INFO(MauCor::LogCategory::Game, "Debug Render Mode: {}", debModeStr);
		}

		if (input.IsActionExecuted("RandomizeLightColours"))
		{
			ME_LOG_INFO(MauCor::LogCategory::Game, "Randomizing light colours");

			auto view{ GetECSWorld().View<MauEng::CLight>() };
			view.Each([](MauEng::CLight& light)
				{
					std::string lightTypeStr{ "NONE" };
					switch (light.type) {
					case MauEng::ELightType::DIRECTIONAL:
						lightTypeStr = "Directional Light " + std::to_string(light.lightID);
						break;
					case MauEng::ELightType::POINT:
						lightTypeStr = "Point Light " + std::to_string(light.lightID);
						break;

					}

					// Random device for seed 
					std::random_device rd;
					// Mersenne Twister generator
					std::mt19937 gen(rd());
					// Random colour range
					std::uniform_real_distribution<float> dis(0, 1.f);

					light.lightColour= { dis(gen), dis(gen), dis(gen) };
					ME_LOG_INFO(MauCor::LogCategory::Game, "Light Colour ({}): [{:.2f}, {:.2f}, {:.2f}]", lightTypeStr, light.lightColour.x, light.lightColour.y, light.lightColour.z);
				}
			);

		}

		if (input.IsActionExecuted("ToggleCamSettings"))
		{
			uint8_t currModeID{ static_cast<uint8_t>(m_CamSettings) };
			++currModeID;
			currModeID %= static_cast<uint8_t>(ECamSettings::COUNT);
			m_CamSettings = static_cast<ECamSettings>(currModeID);

			std::string camSettStr{ "NONE" };
			switch (m_CamSettings) {
			case ECamSettings::NoExposure:
				camSettStr = "No Exposure";
				GetCameraManager().GetActiveCamera().DisableExposure();
				break;
			case ECamSettings::SUNNY16:
				camSettStr = "Sunny 16";
				GetCameraManager().GetActiveCamera().SetCamSettingsSunny16();
				break;
			case ECamSettings::INDOOR:
				camSettStr = "Indoor";
				GetCameraManager().GetActiveCamera().SetCamSettingsIndoor();
				break;
			case ECamSettings::CUSTOM:
				camSettStr = "Custom";
				GetCameraManager().GetActiveCamera().EnableExposure();
				GetCameraManager().GetActiveCamera().SetExposureOverride(1.f);
				break;
			}

			ME_LOG_INFO(MauCor::LogCategory::Game, "Cam Settings: {}", camSettStr);
		}

		if (input.IsActionExecuted("ToggleToneMap"))
		{
			uint8_t currModeID{ static_cast<uint8_t>(m_CameraManager.GetActiveCamera().GetToneMapper()) };
			++currModeID;
			currModeID %= static_cast<uint8_t>(MauEng::Camera::ToneMapper::COUNT);
			m_CameraManager.GetActiveCamera().SetToneMapper(static_cast<MauEng::Camera::ToneMapper>(currModeID));

			std::string camSettStr{ "NONE" };
			switch (m_CameraManager.GetActiveCamera().GetToneMapper()) {
			case MauEng::Camera::ToneMapper::ACESFilm:
				camSettStr = "ACES Film";
				break;
			case MauEng::Camera::ToneMapper::Uncharted2:
				camSettStr = "Uncharted 2";
				break;
			case MauEng::Camera::ToneMapper::Reinhard:
				camSettStr = "Reinhard";
				break;
			case MauEng::Camera::ToneMapper::COUNT:
				break;
			}

			ME_LOG_INFO(MauCor::LogCategory::Game, "Cam Tone mapper: {}", camSettStr);
		}

		if (input.IsActionExecuted("LowerCustomExposure"))
		{
			float curr = GetCameraManager().GetActiveCamera().GetExposureOverride();
			curr *= pow(2.0f, -1.0f / 3.0f); // LOWER exposure by 1/3 stop

			GetCameraManager().GetActiveCamera().SetExposureOverride(curr);

			ME_LOG_INFO(MauCor::LogCategory::Game, "New exposure: {}", curr);
		}

		if (input.IsActionExecuted("HigherCustomExposure"))
		{
			float curr = GetCameraManager().GetActiveCamera().GetExposureOverride();
			curr *= pow(2.0f, 1.0f / 3.0f); // INCREASE exposure by 1/3 stop

			GetCameraManager().GetActiveCamera().SetExposureOverride(curr);

			ME_LOG_INFO(MauCor::LogCategory::Game, "New exposure: {}", curr);
		}

		if (input.IsActionExecuted("PrintInfo"))
		{
			OutputKeybinds();
		}

	}

	void DemoScene::RenderDebugDemo() const
	{
		ME_PROFILE_FUNCTION()

		// Config
		bool const DRAW_LINES{		m_DebugRenderMode == EDebugRenderMode::ALL or
									m_DebugRenderMode == EDebugRenderMode::DRAW_LINES };
		bool const DRAW_RECTS{		m_DebugRenderMode == EDebugRenderMode::ALL or
									m_DebugRenderMode == EDebugRenderMode::DRAW_RECTS };
		bool const DRAW_TRIANGLES{	m_DebugRenderMode == EDebugRenderMode::ALL or
									m_DebugRenderMode == EDebugRenderMode::DRAW_TRIANGLES };
		bool const DRAW_ARROWS{	 m_DebugRenderMode == EDebugRenderMode::ALL or
								 m_DebugRenderMode == EDebugRenderMode::DRAW_ARROWS };
		bool const DRAW_CIRCLES{ m_DebugRenderMode == EDebugRenderMode::ALL or
								 m_DebugRenderMode == EDebugRenderMode::DRAW_CIRCLES };
		bool const DRAW_SPHERES{ m_DebugRenderMode == EDebugRenderMode::ALL or
								 m_DebugRenderMode == EDebugRenderMode::DRAW_SPHERES };
		bool const DRAW_CYL{	 m_DebugRenderMode == EDebugRenderMode::ALL or
								 m_DebugRenderMode == EDebugRenderMode::DRAW_CYL };
		bool const DRAW_POLY{	 m_DebugRenderMode == EDebugRenderMode::ALL or
								 m_DebugRenderMode == EDebugRenderMode::DRAW_POLY };

		// Demo debug drawing tests
		if (DRAW_LINES)
		{
			DEBUG_RENDERER.DrawLine({ 0, 0,0 }, { 100, 10, 10 }, { 0, 0, 0 });
			DEBUG_RENDERER.DrawLine({ 0, 0,0 }, { 100, 10, 10 }, { 90, 0, 0 }, { 0, 1, 0 });
			DEBUG_RENDERER.DrawLine({ 0, 0,0 }, { 100, 10, 10 }, { 180, 0, 0 }, { 0, 1, 0 });
			DEBUG_RENDERER.DrawLine({ 0, 0,0 }, { 100, 10, 10 }, { 0, 90, 0 }, { 0, 0, 1 });
			DEBUG_RENDERER.DrawLine({ 0, 0,0 }, { 100, 10, 10 }, { 0, 180, 0 }, { 0, 0, 1 });
			DEBUG_RENDERER.DrawLine({ 0, 0,0 }, { 100, 10, 10 }, { 0, 0, 90 }, { 0, 1, 1 });
			DEBUG_RENDERER.DrawLine({ 0, 0,0 }, { 100, 10, 10 }, { 0, 0, 180 }, { 0, 1, 1 });

			static float constexpr ROT_SPEED{ 10.f };
			static float lineRot{};
			DEBUG_RENDERER.DrawLine({ 0, 0,0 }, { 100, 0, 0 }, { lineRot, lineRot, lineRot }, { 1, 1, 1 });

			DEBUG_RENDERER.DrawLine({ 0, 0,0 }, { 10, 0, 0 }, { 0, lineRot, 0 }, { 1, 1, 0 });
			DEBUG_RENDERER.DrawLine({ 0, 0,0 }, { 10, 0, 0 }, { 0, lineRot, 0 }, { 1, 1, 1 }, true, { 100, 100, 100 });
			DEBUG_RENDERER.DrawLine({ 0, 0,0 }, { 10, 0, 0 }, { 0, 0, lineRot }, { 1, 1, 0 });

			lineRot += ROT_SPEED * TIME.ElapsedSec();
		}

		if (DRAW_RECTS)
		{
			static float constexpr CUBE_ROT_SPEED{ 10.f };
			static float cubeRot{};

			DEBUG_RENDERER.DrawRect({ 0, 0, 0 }, { 20, 30 }, { 0, 0, cubeRot }, { 1, 0, 0 });
			DEBUG_RENDERER.DrawRect({ 0, 0, 0 }, { 20, 30 }, { 0, 0, 0 }, { 1, 1, 1 });
			DEBUG_RENDERER.DrawCube({ 20, 20, 20 }, { 69, 20, 20 }, { 0, 0, 0 }, { 1, 1, 1 });
			DEBUG_RENDERER.DrawCube({ 20, 20, 20 }, { 69, 20, 20 }, { 0, cubeRot, 0 });

			DEBUG_RENDERER.DrawRect({ 0, 0, 0 }, { 20, 30 }, { 0, 0, cubeRot }, { 0, 1, 1 }, true, { 20, 20, 20 });
			DEBUG_RENDERER.DrawCube({ 20, 20, 20 }, { 69, 20, 20 }, { 0, cubeRot, 0 }, { 1, 1,0 }, true, { 0, 0, 0 });


			cubeRot += CUBE_ROT_SPEED * TIME.ElapsedSec();
		}

		if (DRAW_TRIANGLES)
		{
			static float constexpr TRIANGLE_ROT_SPEED{ 10.f };
			static float triRot{};

			DEBUG_RENDERER.DrawTriangle({ 0,0,0 }, { 10, 10, 10 }, { 20, 10, 16 });
			DEBUG_RENDERER.DrawTriangle({ 0,0,0 }, { 10, 10, 10 }, { 20, 10, 16 }, { 0, triRot, triRot }, { 1, 1, 1 });

			DEBUG_RENDERER.DrawTriangle({ 0,0,0 }, { 10, 10, 10 }, { 20, 10, 16 }, { 0, triRot, triRot }, { 1, 1, 1 }, true, { 25,25, 0 });


			triRot += TRIANGLE_ROT_SPEED * TIME.ElapsedSec();
		}

		if (DRAW_ARROWS)
		{
			static float constexpr ARROW_ROT_SPEED{ 10.f };
			static float arrRot{};

			DEBUG_RENDERER.DrawArrow({}, { 2, 3, 3 });
			DEBUG_RENDERER.DrawArrow({}, { 2, 3, 3 }, { 0, arrRot, 0 }, { 1, 1, 1 });

			DEBUG_RENDERER.DrawArrow({}, { 0, 0, 3 });
			DEBUG_RENDERER.DrawArrow({}, { 0, 0, 3 }, { arrRot,0 , 0 }, { 1, 1, 1 });

			DEBUG_RENDERER.DrawArrow({}, { 0, 0, 3 }, { arrRot,0 , 0 }, { 1, 1, 1 }, 1, true, { 20, 20, 0 });

			arrRot += ARROW_ROT_SPEED * TIME.ElapsedSec();
		}

		if (DRAW_CIRCLES)
		{
			static float constexpr CIRCLE_ROT_SPEED{ 10.f };
			static float circleRot{};

			DEBUG_RENDERER.DrawCircle({}, 20);
			DEBUG_RENDERER.DrawCircle({}, 20, { circleRot, 0, 0 }, { 1, 1, 1 });

			DEBUG_RENDERER.DrawEllipse({}, { 100, 10 }, {}, { 0, 1, 0 });
			DEBUG_RENDERER.DrawEllipse({}, { 100, 10 }, { circleRot }, { 1, 1, 1 });

			DEBUG_RENDERER.DrawEllipse({}, { 100, 10 }, { circleRot }, { 1, 1, 1 }, 24, true, { 20, 20, 0 });

			circleRot += CIRCLE_ROT_SPEED * TIME.ElapsedSec();
		}

		if (DRAW_SPHERES)
		{
			static float constexpr SPHERE_ROT_SPEED{ 10.f };
			static float sphereRot{};

			DEBUG_RENDERER.DrawSphere({}, 20.f);
			DEBUG_RENDERER.DrawSphere({}, 20.f, sphereRot, { 0, 0,1 });

			DEBUG_RENDERER.DrawSphere({ -10, -10, -10 }, 20.f, {}, { 1, 1, 0 });
			DEBUG_RENDERER.DrawSphere({ -10, -10, -10 }, 20.f, sphereRot, { 1,1,1 });

			DEBUG_RENDERER.DrawSphereComplex({ 20,20,20 }, 20.f, { }, { 1, 0, 0 }, 24, 6);
			DEBUG_RENDERER.DrawSphereComplex({ 20,20,20 }, 20.f, { sphereRot }, { 1, 1, 1 }, 24, 6);

			DEBUG_RENDERER.DrawEllipsoid({ -30, -30, 0 }, { 10, 20, 20 }, {});
			DEBUG_RENDERER.DrawEllipsoid({ -30, -30, 0 }, { 10, 20, 20 }, { sphereRot }, { 1, 1, 1 });

			DEBUG_RENDERER.DrawEllipsoidComplex({ 0, 0, 50 }, { 20, 50, 20 }, {}, { 1, 0, 0 }, 100, 30);
			DEBUG_RENDERER.DrawEllipsoidComplex({ 0, 0, 50 }, { 20, 50, 20 }, { sphereRot }, { 1, 1, 1 }, 100, 30);

			DEBUG_RENDERER.DrawSphere({ 20, 20, 20 }, 1, {}, { .5, .5, 1 });

			DEBUG_RENDERER.DrawEllipsoidComplex({ 0, 0, -50 }, { 20, 50, 20 }, {}, { 1, 0, 1 }, 100, 30, false);
			DEBUG_RENDERER.DrawEllipsoidComplex({ 0, 0, -50 }, { 20, 50, 20 }, {}, { 1, 1, 1 }, 100, 30, true);
			DEBUG_RENDERER.DrawEllipsoidComplex({ 0, 0, -50 }, { 20, 50, 20 }, {}, { 1, 1, 1 }, 100, 30, true, { 20, 20, 20 });
			DEBUG_RENDERER.DrawEllipsoidComplex({ 0, 0, -50 }, { 20, 50, 20 }, { 0, sphereRot }, { 1, 1, 0 }, 100, 30, true, { 20, 20, 20 });


			sphereRot += SPHERE_ROT_SPEED * TIME.ElapsedSec();
		}

		if (DRAW_CYL)
		{
			static float constexpr CYL_ROT_SPEED{ 10.f };
			static float cylRot{};

			DEBUG_RENDERER.DrawCylinder({ -10, 0, -10 }, { 20, 100, 20 });
			DEBUG_RENDERER.DrawCylinder({ -10, 0, -10 }, { 20, 100, 20 }, { cylRot, cylRot, cylRot }, { 1, 1, 1 });
			DEBUG_RENDERER.DrawCylinder({ -10, 0, -10 }, { 20, 100, 20 }, { cylRot, cylRot, cylRot }, { 0, 1, 0 }, 24, true, { 20, 20, 20 });
			DEBUG_RENDERER.DrawCylinder({ -10, 0, -10 }, { 20, 100, 20 }, {}, { 0, 1, 1 }, 24, true, { 20, 20, 20 });

			cylRot += CYL_ROT_SPEED * TIME.ElapsedSec();
		}

		if (DRAW_POLY)
		{
			static float constexpr POL_ROT_SPEED{ 10.f };
			static float polRot{};

			DEBUG_RENDERER.DrawSphere({}, 1);

			DEBUG_RENDERER.DrawPolygon({ {20, 0, 0}, { 30, 0, 0 }, {30, 10, 0}, {20, 10, 0} });
			DEBUG_RENDERER.DrawPolygon({ {20, 0, 0}, { 30, 0,0 }, {30, 10, 0}, {20, 10, 0} }, { 0, polRot }, { 1, 1, 1 });

			DEBUG_RENDERER.DrawPolygon({ {20, 0, 0}, { 30, 0, 0  }, {30, 10, 0 }, {20, 10, 0} }, { 0, polRot }, { 1, 1, 1 }, true);


			polRot += POL_ROT_SPEED * TIME.ElapsedSec();
		}
	}

	void DemoScene::OutputKeybinds()
	{
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "Demo Scene Info");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "Key binds: ");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "SPACE: Print this screen");

		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "F1: Profile");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "F2: Toggle light debug rendering");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "F3: Toggle light mode - point light only; dir light only, both");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "F4: Toggle shadows");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "F5: Lower light intensity");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "F6: Higher light intensity");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "F7: Toggle scene rotation");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "F8: Toggle Debug render mode");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "F9: Randomize light colours");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "F10: Toggle Cam settings");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "F11: Toggle Tone map\n");

		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "E: Lower exposure (custom exposure mode)");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "R: Higher exposure (custom exposure mode)\n");

		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "WASD | ARROWS: Move Camera");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "Mouse movement: Rotate Camera");
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "Left control: \"Sprint\"");
	}
}
