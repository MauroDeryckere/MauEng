#include "DemoScene.h"

#include <random>
#include "CustomPlayerClass.h"

#include "Components/CDebugText.h"

namespace MauGam
{
	DEFINE_LOG_CATEGORY(TestLogCategory)
	DEFINE_LOG_CATEGORY(TestLogCategory2, Warn)
	DEFINE_LOG_CATEGORY(TestTimers)

	DemoScene::DemoScene()
	{
		ME_PROFILE_FUNCTION()

		// Testing the custom log categories
		ME_LOG(Debug, TestLogCategory, "TEST");
		ME_LOG(Debug, TestLogCategory2, "TEST CAT 02 - DEBUG");
		ME_LOG(Warn, TestLogCategory2, "TEST CAT 02 - WARN");
		TestLogCategory2.SetPriority(Debug);
		ME_LOG(Debug, TestLogCategory2, "TEST CAT 02 - DEBUG");
		ME_LOG(Warn, TestLogCategory2, "TEST CAT 02 - WARN");
	}

	void DemoScene::OnLoad()
	{
		ME_PROFILE_FUNCTION()

		Scene::OnLoad();

		using namespace MauEng;

#pragma region TimerTest
		{
			{
				// Set a one-shot timer (2 seconds)
				auto const& handle2{ m_TimerManager += MauCor::TimerDataCallable{ MauCor::Bind([]()
					{
						ME_LOG_DEBUG(TestTimers, "Timer 2 fired (one-shot after 2s)");
					}), 2.f} };

				// Set a looping timer (1 second)
				auto const& handle1{ m_TimerManager.SetTimer([&]()
					{
						ME_LOG_DEBUG(TestTimers, "Timer 1 fired (looping every 1s)");
						m_TimerManager.RemoveTimer(handle2);
					}, 1.f, true) };

				m_TimerManager %= (handle1);

				if (m_TimerManager.IsTimerActive(handle1))
				{
					ME_LOG_DEBUG(TestTimers, "Timer 1 is active");
				}
				else
				{
					ME_LOG_DEBUG(TestTimers, "Timer 1 is NOT active");
				}
				if (m_TimerManager.IsTimerActive(handle2))
				{
					ME_LOG_DEBUG(TestTimers, "Timer 2 is active");
				}
				else
				{
					ME_LOG_DEBUG(TestTimers, "Timer 2 is NOT active");
				}
			}

			{
				m_TimerManager.SetTimer([&]() { ME_LOG_DEBUG(TestTimers, "Timer 3 fired"); m_TimerManager.RemoveAllTimers(this); }, 0.5f, false, this);
				m_TimerManager.SetTimer([]() { ME_LOG_DEBUG(TestTimers, "Timer 3 fired");  }, 1.f, false, this);
				m_TimerManager.SetTimer([]() { ME_LOG_DEBUG(TestTimers, "Timer 3 fired");  }, 1.f, false, this);
				m_TimerManager.SetTimer([]() { ME_LOG_DEBUG(TestTimers, "Timer 3 fired");  }, 1.f, false, this);
				m_TimerManager.SetTimer([]() { ME_LOG_DEBUG(TestTimers, "Timer 3 fired");  }, 1.f, false, this);
			}

			{
				m_TimerManager.SetTimer([&]()
				{
					ME_LOG_DEBUG(TestTimers, "Setup member function timers");
					m_TimerManager.SetTimer(&DemoScene::OnTimerFires, this, 5.f, true);
					m_TimerManager.SetTimer(&DemoScene::OnTimerFiresConst, this, 5.f, true);

					m_TimerManager.SetTimer([&]() { ME_LOG_DEBUG(TestTimers, "Clearing timers"); m_TimerManager.RemoveAllTimers(this); }, 11.f, false, this);
				}, 5.f);
			}

			{
				// Next ticks
				m_TimerManager *= MauCor::Bind([&]()
					{
						ME_LOG_DEBUG(TestTimers, "Next tick timer fired");
					});
				m_TimerManager.SetTimerForNextTick(&DemoScene::OnTimerFires, this);
				m_TimerManager.SetTimerForNextTick(&DemoScene::OnTimerFiresConst, this);
			}
		}
#pragma endregion

#pragma region EventTest
		// eventtype
		{
			auto const& handle{ m_DelegateTest += MauCor::Bind<TestEvent>
			(
				[this](TestEvent const& event) { OnDelegate(event); }
			) };

			TestEvent event{};

			m_DelegateTest.Get()->Broadcast(event);
			m_DelegateTest -= handle;
			m_DelegateTest < event;

			event.i = 20;
			auto const& handle02{ m_DelegateTest += MauCor::Bind<TestEvent>([this](TestEvent const& event) { OnDelegate(event); }, this) };
			m_DelegateTest.Broadcast(event);
			m_DelegateTest.UnSubscribeImmediate(handle02.owner);
			m_DelegateTest < event;

			event.i = 30;
			m_DelegateTest.Get()->Subscribe([this](TestEvent const& event) { OnDelegate(event); }, this);
			m_DelegateTest.Broadcast(event);
			m_DelegateTest /= this;
			m_DelegateTest < event;

			event.i = 40;
			m_DelegateTest += MauCor::Bind(&DemoScene::OnDelegate, this);
			m_DelegateTest.Broadcast(event);
			m_DelegateTest /= this;
			m_DelegateTest.Broadcast(event);

			event.i = 50;
			m_DelegateTest += MauCor::Bind(&DemoScene::OnDelegateConst, this);
			m_DelegateTest.Broadcast(event);
			m_DelegateTest.UnSubscribeAllByOwnerImmediate(this);
			m_DelegateTest.Broadcast(event);

			event.i = 60;
			m_DelegateTest += MauCor::Bind(&DemoScene::OnDelegateConst, this);
			m_DelegateTest << event;

			m_DelegateTest.Clear();
		}
		// void
		{
			auto const& handle{ m_DelegateVoidTest += MauCor::Bind<void>
			(
				[this]() { OnDelegateVoid(); }
			) };
			m_DelegateVoidTest.Get()->Broadcast();
			m_DelegateVoidTest /= handle;
			m_DelegateVoidTest.Broadcast();

			m_DelegateVoidTest.Get()->Subscribe([this]() { OnDelegateVoid(); }, this);
			m_DelegateVoidTest.Broadcast();
			m_DelegateVoidTest /= this;
			m_DelegateVoidTest.Broadcast();

			m_DelegateVoidTest.Get()->Subscribe(&DemoScene::OnDelegateVoid, this);
			m_DelegateVoidTest.Get()->Subscribe(&DemoScene::OnDelegateConstVoid, this);

			auto bind = MauCor::Bind(&DemoScene::OnDelegateVoid, this);

			m_DelegateVoidTest += bind;
			m_DelegateVoidTest += MauCor::Bind(&DemoScene::OnDelegateConstVoid, this);

			m_DelegateVoidTest.Broadcast();
			m_DelegateVoidTest /= this;
			m_DelegateVoidTest.Broadcast();
			m_DelegateVoidTest.Clear();
		}
#pragma endregion

#pragma region MeshLightAndCamera
		switch (m_Demo)
		{
		case EDemo::Sponza:
		{
			m_CameraManager.GetActiveCamera()->SetPosition({ 0, 20, 10 });
			m_CameraManager.GetActiveCamera()->SetFOV(60.f);

			m_CameraManager.GetActiveCamera()->Focus({ 1,1, 3 });
			m_CameraManager.GetActiveCamera()->SetFar(1000);

			{
				Entity enttSponza{ CreateEntity() };

				auto& transform{ enttSponza.GetComponent<CTransform>() };
				float constexpr SCALE{ 10.f };
				transform.Scale({ SCALE, SCALE, SCALE });

				enttSponza.AddComponent<CStaticMesh>("Resources/Models/Sponza/glTF/Sponza.gltf");
			}


			{
				Entity enttDirLight{ CreateEntity({ -1, -.5, -1 }) };
				auto& cLight{ enttDirLight.AddComponent<CLight>() };
				cLight.lumen_lux = 100;
				cLight.direction_position = { -1, -.5, -1 };
				cLight.castShadows = false;
				cLight.lightColour = { 1, 0.956, 0.84 };

				enttDirLight.AddComponent<CDebugText>(std::to_string(cLight.lumen_lux));
			}

			//{
			//	Entity enttPLight{ CreateEntity() };
			//	auto& cLight{ enttPLight.AddComponent<CLight>() };
			//	cLight.type = ELightType::POINT;
			//	cLight.lumen_lux = 1'000'000.f;
			//	cLight.direction_position = { 10, 40, 10 };
			//	cLight.lightColour = { 1, 0, 0 };
			//}

		}
		break;
		case EDemo::Chess:
		{
			m_CameraManager.GetActiveCamera()->SetPosition({ -75, 65, -75 });
			m_CameraManager.GetActiveCamera()->SetFOV(60.f);

			m_CameraManager.GetActiveCamera()->Focus({ 0,10, 0 });
			m_CameraManager.GetActiveCamera()->SetFar(500);

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
				cLight.lumen_lux = 100;
				cLight.direction_position = { -1, -1, -.5 };
				cLight.castShadows = false;
				cLight.lightColour = { 1, 1, .9 };
			}

			{
				Entity enttPLight{ CreateEntity() };
				auto& cLight{ enttPLight.AddComponent<CLight>() };
				cLight.type = ELightType::POINT;
				cLight.lumen_lux = 1'000'000.f;
				cLight.direction_position = { 10, 40, 10 };
				cLight.lightColour = { 1, 0, 0 };
			}
		}
		break;
		case EDemo::FlightHelmet:
		{
			m_CameraManager.GetActiveCamera()->SetPosition({ 0, 50, -100 });
			m_CameraManager.GetActiveCamera()->SetFOV(60.f);

			m_CameraManager.GetActiveCamera()->Focus({ 1,1, 3 });
			m_CameraManager.GetActiveCamera()->SetFar(1000);

			m_CamSettings = ECamSettings::INDOOR;
			m_CameraManager.GetActiveCamera()->SetCamSettingsIndoor();

			{
				Entity enttHelmet{ CreateEntity() };

				auto& transform{ enttHelmet.GetComponent<CTransform>() };
				transform.Scale({ 100.f, 100.f, 100.f });
				enttHelmet.AddComponent<CStaticMesh>("Resources/Models/FlightHelmet/glTF/FlightHelmet.gltf");
			}

			{
				Entity enttDirLight{ CreateEntity({0, 100.f, 0 }) };
				auto& cLight{ enttDirLight.AddComponent<CLight>() };
				cLight.lumen_lux = 100;
				cLight.direction_position = { 0, -1, 0 };
				cLight.castShadows = false;
				cLight.lightColour = { 1, 1, 1 };

				enttDirLight.AddComponent<CDebugText>(std::to_string(cLight.lumen_lux));
			}

			{
				Entity enttPLight{ CreateEntity({ 50.f, 50, -20 }) };
				auto& cLight{ enttPLight.AddComponent<CLight>() };
				cLight.type = ELightType::POINT;
				cLight.lumen_lux = 1'000'000.f;
				cLight.direction_position = { 50.f, 50, -20 };
				cLight.lightColour = { 1, 0, 0 };

				enttPLight.AddComponent<CDebugText>(std::to_string(cLight.lumen_lux));
			}

			{
				Entity enttPLight{ CreateEntity({ -50.f, 50, -20 }) };
				auto& cLight{ enttPLight.AddComponent<CLight>() };
				cLight.type = ELightType::POINT;
				cLight.lumen_lux = 1'000'000.f;
				cLight.direction_position = { -50.f, 50, -20 };
				cLight.lightColour = { 0, 0, 1 };

				enttPLight.AddComponent<CDebugText>(std::to_string(cLight.lumen_lux));
			}
			SetSceneAABBOverride({ -100, -100, -100 }, { 100, 100, 100 });
		}
		break;
		case EDemo::InstanceTest:
		{
			m_CameraManager.GetActiveCamera()->SetPosition({ -50, 50, -50 });
			m_CameraManager.GetActiveCamera()->SetFOV(60.f);

			m_CameraManager.GetActiveCamera()->Focus({ 0, 0, 0 });
			m_CameraManager.GetActiveCamera()->SetFar(1000);

			m_CameraManager.GetActiveCamera()->SetCamSettingsSunny16();


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
				float const fishScale{ disScale(gen) };
				Entity entFish{ CreateEntity() };
				auto& transform{ entFish.GetComponent<CTransform>() };
				transform.Translate({ dis(gen), dis(gen), dis(gen) });
				transform.Scale({ fishScale, fishScale, fishScale });
				entFish.AddComponent<CStaticMesh>("Resources/Models/BarramundiFish/glTF/BarramundiFish.gltf");
			}

			{
				Entity enttDirLight{ CreateEntity() };
				auto& cLight{ enttDirLight.AddComponent<CLight>() };
				cLight.lumen_lux = 20'000.f;
				cLight.direction_position = { -1, -1, -1 };
				cLight.castShadows = false;
				cLight.lightColour = { 0.2f, 0.2f, 1 };
			}
		}
		break;
		case EDemo::DebugRendering:
		{
			m_CameraManager.GetActiveCamera()->SetPosition({ -100, 100, -100 });
			m_CameraManager.GetActiveCamera()->SetFOV(60.f);

			m_CameraManager.GetActiveCamera()->Focus({ 1,1, 3 });
			m_CameraManager.GetActiveCamera()->SetFar(800);


			{
				Entity enttDirLight{ CreateEntity() };
				auto& cLight{ enttDirLight.AddComponent<CLight>() };
				cLight.lumen_lux = 500.f;
				cLight.direction_position = { -1, -1, .1f };
				cLight.castShadows = false;
				cLight.lightColour = { .7, .7, 1 };
			}

			SetSceneAABBOverride({ -300, -300, -300 }, { 300, 300, 300 });
		}
		break;
	case EDemo::COUNT:
		break;
	default:
		break;
		}
#pragma endregion

		SetupInput();
		OutputKeybinds();
	}

	void DemoScene::Tick()
	{
		ME_PROFILE_FUNCTION()

		Scene::Tick();

		// runtime model load test
		using namespace MauEng;
		static bool didOnce = false;
		if (not didOnce)
		{
			Entity entFish{ CreateEntity() };
			auto& transform{ entFish.GetComponent<CTransform>() };
			transform.Scale({ 100, 100, 100 });
			transform.Translate({ -50, 0, -50 });
			entFish.AddComponent<CStaticMesh>("Resources/Models/BarramundiFish/glTF/BarramundiFish.gltf");

			entFish.Destroy();
		}
		didOnce = true;


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
				auto view{ GetECSWorld().View<CStaticMesh, CTransform>() };
				view.Each([&rot](CStaticMesh const& m, CTransform& t)
					{
						t.Rotate(rot);
					}, std::execution::par_unseq);
			}
			else
			{
				using namespace MauEng;
				float constexpr ROTATION_SPEED{ 15.f };
				MauCor::Rotator const rot{ 0, ROTATION_SPEED * TIME.ElapsedSec() };
				auto view{ GetECSWorld().View<CStaticMesh, CTransform>() };
				view.Each([&rot](CStaticMesh const& m, CTransform& t)
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
			auto view{ GetECSWorld().View<MauEng::CTransform, MauEng::CLight>() };
			view.Each([](MauEng::CTransform const& t, MauEng::CLight const& l)
				{
					switch (l.type)
					{
					case MauEng::ELightType::DIRECTIONAL:
					{
						glm::vec3 const start{ t.translation };
						glm::vec3 const dir{ glm::normalize(l.direction_position) };
						float const length{ std::clamp(l.lumen_lux / 10000.f, 5.f, 40.f) };
						glm::vec3 const end{ start + dir * length };

						DEBUG_RENDERER.DrawArrow(start, end, {}, l.lightColour, 1.f);

						break;
					}

					case MauEng::ELightType::POINT:
						DEBUG_RENDERER.DrawSphere(l.direction_position, std::clamp(l.lumen_lux / 10000.f, 2.f, 20.f), {}, l.lightColour);
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
		input.DestroyPlayer(0u);
		input.CreatePlayer<PlayerClass>();

		auto const player{ input.GetPlayer() };

		input.BindAction("PrintInfo", MauEng::KeyInfo{ SDLK_SPACE, MauEng::KeyInfo::ActionType::Up });

		input.BindAction("PrintInfo", MauEng::KeyInfo{ SDLK_V, MauEng::KeyInfo::ActionType::Up }, "SECONDCONTEXTTEST");
		player->SetMappingContext("SECONDCONTEXTTEST");

		input.EraseMappingContext("SECONDCONTEXTTEST", "DEFAULT");

		input.BindAction("ToggleLightDebugRendering", MauEng::KeyInfo{ SDLK_F2, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleLights", MauEng::KeyInfo{ SDLK_F3, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleShadows", MauEng::KeyInfo{ SDLK_F4, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("DownLightIntensity", MauEng::KeyInfo{ SDLK_F5, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("UpLightIntensity", MauEng::KeyInfo{ SDLK_F6, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleRotation", MauEng::KeyInfo{ SDLK_F7, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleDebugRenderMode", MauEng::KeyInfo{ SDLK_F8, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("RandomizeLightColours", MauEng::KeyInfo{ SDLK_F9, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleCamSettings", MauEng::KeyInfo{ SDLK_F10, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("ToggleCamSettings", MauEng::GamepadInfo{ SDL_GAMEPAD_BUTTON_SOUTH, MauEng::GamepadInfo::ActionType::Up });
		//input.UnBindAction("ToggleCamSettings");

		input.BindAction("ToggleToneMap", MauEng::KeyInfo{ SDLK_F11, MauEng::KeyInfo::ActionType::Up });

		input.BindAction("LowerCustomExposure", MauEng::KeyInfo{ SDLK_E, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("HigherCustomExposure", MauEng::KeyInfo{ SDLK_R, MauEng::KeyInfo::ActionType::Up });
		

		//input.BindAction("MoveUp", MauEng::MouseInfo{ SDL_BUTTON_LEFT, MauEng::MouseInfo::ActionType::Down });
		input.BindAction("MoveUp", MauEng::KeyInfo{ SDLK_UP, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveLeft", MauEng::KeyInfo{ SDLK_LEFT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveRight", MauEng::KeyInfo{ SDLK_RIGHT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveDown", MauEng::KeyInfo{ SDLK_DOWN, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveUp", MauEng::KeyInfo{ SDLK_W, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveLeft", MauEng::KeyInfo{ SDLK_A, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveRight", MauEng::KeyInfo{ SDLK_D, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveDown", MauEng::KeyInfo{ SDLK_S, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("MoveUp", MauEng::GamepadInfo{ SDL_GAMEPAD_BUTTON_DPAD_UP, MauEng::GamepadInfo::ActionType::Held });
		input.BindAction("MoveLeft", MauEng::GamepadInfo{ SDL_GAMEPAD_BUTTON_DPAD_LEFT, MauEng::GamepadInfo::ActionType::Held });
		input.BindAction("MoveRight", MauEng::GamepadInfo{ SDL_GAMEPAD_BUTTON_DPAD_RIGHT, MauEng::GamepadInfo::ActionType::Held });
		input.BindAction("MoveDown", MauEng::GamepadInfo{ SDL_GAMEPAD_BUTTON_DPAD_DOWN, MauEng::GamepadInfo::ActionType::Held });

		input.BindAction("MoveGamepadX", MauEng::GamepadInfo{ .input= { .axis=SDL_GAMEPAD_AXIS_LEFTX }, .type = MauEng::GamepadInfo::ActionType::AxisHeld });
		input.BindAction("MoveGamepadY", MauEng::GamepadInfo{ .input = { .axis = SDL_GAMEPAD_AXIS_LEFTY }, .type = MauEng::GamepadInfo::ActionType::AxisHeld });

		input.BindAction("AxisReleasedTest", MauEng::GamepadInfo{ .input = { .axis = SDL_GAMEPAD_AXIS_LEFT_TRIGGER }, .type = MauEng::GamepadInfo::ActionType::AxisReleased });
		input.BindAction("AxisStartHeldTest", MauEng::GamepadInfo{ .input = { .axis = SDL_GAMEPAD_AXIS_LEFT_TRIGGER }, .type = MauEng::GamepadInfo::ActionType::AxisStartHeld });

		input.BindAction("Sprint", MauEng::KeyInfo{ SDLK_LCTRL, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("Rotate", MauEng::MouseInfo{ {}, MauEng::MouseInfo::ActionType::Moved });
		input.BindAction("LeftMBHeld", MauEng::MouseInfo{ {SDL_BUTTON_LEFT}, MauEng::MouseInfo::ActionType::Held });

		input.BindAction("SpawnFish", MauEng::KeyInfo{ {SDLK_V }, MauEng::KeyInfo::ActionType::Up });
		input.BindAction("DeleteFish", MauEng::KeyInfo{ {SDLK_B}, MauEng::KeyInfo::ActionType::Up });

		//Unbind tests
		//input.UnBindAction("PrintInfo");
		//input.UnBindAllActions(MauEng::KeyInfo{ SDLK_UP,MauEng::KeyInfo::ActionType::Held });
		//input.UnBindAllActions(MauEng::MouseInfo{ {},MauEng::MouseInfo::ActionType::Moved });
	}

	void DemoScene::HandleInput()
	{
		ME_PROFILE_FUNCTION()

		auto const& input{ INPUT_MANAGER };

		bool isSprinting{ false };
		auto constexpr sprintModifier{ 6.f };
		auto constexpr movementSpeed{ 20.f };

		auto const player{ input.GetPlayer() };

		if (player->IsActionExecuted("Sprint"))
		{
			isSprinting = true;
		}

		if (player->IsActionExecuted("MoveUp"))
		{
			m_CameraManager.GetActiveCamera()->Translate({ 0.f, 0.f, movementSpeed * TIME.ElapsedSec() * (isSprinting ? sprintModifier : 1) });
		}
		if (player->IsActionExecuted("MoveDown"))
		{
			m_CameraManager.GetActiveCamera()->Translate({ 0.f, 0.f, -movementSpeed * TIME.ElapsedSec() * (isSprinting ? sprintModifier : 1) });
		}
		if (player->IsActionExecuted("MoveLeft"))
		{
			m_CameraManager.GetActiveCamera()->Translate({ -movementSpeed * TIME.ElapsedSec() * (isSprinting ? sprintModifier : 1), 0.f, 0.f });
		}
		if (player->IsActionExecuted("MoveRight"))
		{
			m_CameraManager.GetActiveCamera()->Translate({ movementSpeed * TIME.ElapsedSec() * (isSprinting ? sprintModifier : 1), 0.f, 0.f });
		}

		float constexpr mouseRotSpeed{ 60 };
		if (player->IsActionExecuted("Rotate") and player->IsActionExecuted("LeftMBHeld"))
		{
			auto const mouseMovement{ player->GetDeltaMouseMovement() };
			float const rot{ mouseRotSpeed * TIME.ElapsedSec() };

			m_CameraManager.GetActiveCamera()->RotateX(mouseMovement.first * rot);
			m_CameraManager.GetActiveCamera()->RotateY(-mouseMovement.second * rot);
		}

		float constexpr LIGHT_ADJUSTMENT{ 200.f };
		if (player->IsActionExecuted("DownLightIntensity"))
		{
			auto view{ GetECSWorld().View<MauEng::CLight>() };
			view.Each([LIGHT_ADJUSTMENT](MauEng::CLight& light)
				{
					light.lumen_lux -= LIGHT_ADJUSTMENT;
					ME_LOG_INFO(LogGame, "Light intensity: {}", light.lumen_lux);
				});
		}

		if (player->IsActionExecuted("UpLightIntensity"))
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
					light.lumen_lux += LIGHT_ADJUSTMENT;
					ME_LOG_INFO(LogGame, "Light intensity ({}): {}", lightTypeStr, light.lumen_lux);
				});
		}

		if (player->IsActionExecuted("ToggleShadows"))
		{
			m_CastShadows = not m_CastShadows;
			ME_LOG_INFO(LogGame, "Shadows: {}", m_CastShadows);

			auto view{ GetECSWorld().View<MauEng::CLight>() };
			view.Each([this](MauEng::CLight& light)
				{
					light.castShadows = m_CastShadows;
				});
		}
		
		if (player->IsActionExecuted("ToggleLights"))
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
			ME_LOG_INFO(LogGame, "Light mode: {}", lightMode);

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

		if (player->IsActionExecuted("ToggleLightDebugRendering"))
		{
			m_DebugRenderLight = not m_DebugRenderLight;
			ME_LOG_INFO(LogGame, "Debug render light: {}", m_DebugRenderLight);
		}

		if (player->IsActionExecuted("ToggleRotation"))
		{
			m_Rotate = not m_Rotate;
			ME_LOG_INFO(LogGame, "Scene rotation: {}", m_Rotate);
		}

		if (player->IsActionExecuted("ToggleDebugRenderMode"))
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

			ME_LOG_INFO(LogGame, "Debug Render Mode: {}", debModeStr);
		}

		if (player->IsActionExecuted("RandomizeLightColours"))
		{
			ME_LOG_INFO(LogGame, "Randomizing light colours");

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
					ME_LOG_INFO(LogGame, "Light Colour ({}): [{:.2f}, {:.2f}, {:.2f}]", lightTypeStr, light.lightColour.x, light.lightColour.y, light.lightColour.z);
				}
			);

		}

		if (player->IsActionExecuted("ToggleCamSettings"))
		{
			uint8_t currModeID{ static_cast<uint8_t>(m_CamSettings) };
			++currModeID;
			currModeID %= static_cast<uint8_t>(ECamSettings::COUNT);
			m_CamSettings = static_cast<ECamSettings>(currModeID);

			std::string camSettStr{ "NONE" };
			switch (m_CamSettings) {
			case ECamSettings::NoExposure:
				camSettStr = "No Exposure";
				GetCameraManager().GetActiveCamera()->DisableExposure();
				break;
			case ECamSettings::SUNNY16:
				camSettStr = "Sunny 16";
				GetCameraManager().GetActiveCamera()->SetCamSettingsSunny16();
				break;
			case ECamSettings::INDOOR:
				camSettStr = "Indoor";
				GetCameraManager().GetActiveCamera()->SetCamSettingsIndoor();
				break;
			case ECamSettings::CUSTOM:
				camSettStr = "Custom";
				GetCameraManager().GetActiveCamera()->EnableExposure();
				GetCameraManager().GetActiveCamera()->SetExposureOverride(1.f);
				break;
			}

			ME_LOG_INFO(LogGame, "Cam Settings: {}", camSettStr);
		}

		if (player->IsActionExecuted("ToggleToneMap"))
		{
			uint8_t currModeID{ static_cast<uint8_t>(m_CameraManager.GetActiveCamera()->GetToneMapper()) };
			++currModeID;
			currModeID %= static_cast<uint8_t>(MauEng::Camera::ToneMapper::COUNT);
			m_CameraManager.GetActiveCamera()->SetToneMapper(static_cast<MauEng::Camera::ToneMapper>(currModeID));

			std::string camSettStr{ "NONE" };
			switch (m_CameraManager.GetActiveCamera()->GetToneMapper()) {
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

			ME_LOG_INFO(LogGame, "Cam Tone mapper: {}", camSettStr);
		}

		if (player->IsActionExecuted("LowerCustomExposure"))
		{
			float curr = GetCameraManager().GetActiveCamera()->GetExposureOverride();
			curr *= pow(2.0f, -1.0f / 3.0f); // LOWER exposure by 1/3 stop

			GetCameraManager().GetActiveCamera()->SetExposureOverride(curr);

			ME_LOG_INFO(LogGame, "New exposure: {}", curr);
		}

		if (player->IsActionExecuted("HigherCustomExposure"))
		{
			float curr = GetCameraManager().GetActiveCamera()->GetExposureOverride();
			curr *= pow(2.0f, 1.0f / 3.0f); // INCREASE exposure by 1/3 stop

			GetCameraManager().GetActiveCamera()->SetExposureOverride(curr);

			ME_LOG_INFO(LogGame, "New exposure: {}", curr);
		}

		if (player->IsActionExecuted("PrintInfo"))
		{
			OutputKeybinds();
		}

		if (player->IsActionExecuted("MoveGamepadX") or player->IsActionExecuted("MoveGamepadY"))
		{
			auto const& lJoy{ player->GetLeftJoystick() };
			auto const& x{ lJoy.first };
			auto const& y{ lJoy.second };

			m_CameraManager.GetActiveCamera()->Translate({ 0.f, 0.f, -y * movementSpeed * TIME.ElapsedSec() * (isSprinting ? sprintModifier : 1) });
			m_CameraManager.GetActiveCamera()->Translate({ x * movementSpeed * TIME.ElapsedSec() * (isSprinting ? sprintModifier : 1), 0.f, 0.f });

		}

		if (player->IsActionExecuted("AxisReleasedTest"))
		{
			ME_LOG_DEBUG(LogGame, "Left trigger axis released");
		}

		if (player->IsActionExecuted("AxisStartHeldTest"))
		{
			ME_LOG_DEBUG(LogGame, "Left trigger axis start hold");
		}

		if (player->IsActionExecuted("SpawnFish"))
		{
			// Random device for seed 
			std::random_device rd;
			// Mersenne Twister generator
			std::mt19937 gen(rd());
			// Random translation range
			std::uniform_real_distribution<float> dis(-300.0f, 300);

			float constexpr FISH_SCALE_MIN{ 10.f };
			float constexpr FISH_SCALE_MAX{ 20.f };
			std::uniform_real_distribution<float> disScale(FISH_SCALE_MIN, FISH_SCALE_MAX);
			float const fishScale{ disScale(gen) };
			MauEng::Entity entFish{ CreateEntity() };
			auto& transform{ entFish.GetComponent<MauEng::CTransform>() };
			transform.Translate({ dis(gen), dis(gen), dis(gen) });
			transform.Scale({ fishScale, fishScale, fishScale });
			entFish.AddComponent<MauEng::CStaticMesh>("Resources/Models/BarramundiFish/glTF/BarramundiFish.gltf");

			m_Fishes.emplace_back(entFish);
		}

		if (player->IsActionExecuted("DeleteFish"))
		{
			ME_LOG_ERROR(LogGame, "EXECUTED DELETE FISH");
			if (not m_Fishes.empty())
			{
				m_Fishes.back().Destroy();
				m_Fishes.pop_back();
			}
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
		bool const DRAW_ARROWS{		m_DebugRenderMode == EDebugRenderMode::ALL or
									m_DebugRenderMode == EDebugRenderMode::DRAW_ARROWS };
		bool const DRAW_CIRCLES{	m_DebugRenderMode == EDebugRenderMode::ALL or
									m_DebugRenderMode == EDebugRenderMode::DRAW_CIRCLES };
		bool const DRAW_SPHERES{	m_DebugRenderMode == EDebugRenderMode::ALL or
									m_DebugRenderMode == EDebugRenderMode::DRAW_SPHERES };
		bool const DRAW_CYL{		m_DebugRenderMode == EDebugRenderMode::ALL or
									m_DebugRenderMode == EDebugRenderMode::DRAW_CYL };
		bool const DRAW_POLY{		m_DebugRenderMode == EDebugRenderMode::ALL or
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
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "Demo Scene Info");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "Key binds: ");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "SPACE: Print this screen");

		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "F1: Profile");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "F2: Toggle light debug rendering");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "F3: Toggle light mode - point light only; dir light only, both");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "F4: Toggle shadows");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "F5: Lower light intensity");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "F6: Higher light intensity");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "F7: Toggle scene rotation");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "F8: Toggle Debug render mode");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "F9: Randomize light colours");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "F10: Toggle Cam settings");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "F11: Toggle Tone map\n");

		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "E: Lower exposure (custom exposure mode)");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "R: Higher exposure (custom exposure mode)\n");

		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "WASD | ARROWS: Move Camera");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "Mouse movement: Rotate Camera");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "Left control: \"Sprint\"\n");

		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "V: Spawn fish");
		LOGGER.Log(MauCor::ELogPriority::Info, LogGame, "B: Delete fish");
	}

	void DemoScene::OnDelegate(TestEvent const& event)
	{
		ME_LOG_DEBUG(LogGame, "Event test: {}", event.i);
	}

	void DemoScene::OnDelegateVoid()
	{
		ME_LOG_DEBUG(LogGame, "Event test (void)");
	}

	void DemoScene::OnDelegateConst(TestEvent const& event) const
	{
		ME_LOG_DEBUG(LogGame, "Event test (const): {}", event.i);
	}

	void DemoScene::OnDelegateConstVoid() const
	{
		ME_LOG_DEBUG(LogGame, "Event test (const) (void)");
	}

	void DemoScene::OnTimerFires()
	{
		ME_LOG_DEBUG(TestTimers, "On Timer fires member function executed");
	}

	void DemoScene::OnTimerFiresConst() const
	{
		ME_LOG_DEBUG(TestTimers, "On Timer fires const member function executed");
	}
}
