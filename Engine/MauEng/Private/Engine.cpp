#include "EnginePCH.h"

#include "Engine.h"

#include <algorithm>
#include <thread>

#include "RendererFactory.h"
#include "Scene/SceneManager.h"
#include "Scene/Camera.h"

#include "Renderer.h"

#include "Window/SDLWindow.h"

#include "glm/glm.hpp"
#include <SDL3/SDL.h>

#include "InternalServiceLocator.h"
#include "Logger/logger.h"

#include "Input/KeyInfo.h"

#include "GUI/ImGUILayer.h"
#include "Logging/ImGUILogger.h"

#include "SoundSystemFactory.h"

namespace MauEng
{
	Engine::Engine():
		m_Window{ std::make_unique<SDLWindow>() }
	{
		// Initialize all core dependences & singletons
		if constexpr (ENABLE_FILE_LOGGING)
		{
			MauCor::CoreServiceLocator::RegisterLogger(MauCor::CreateFileLogger("Log.txt"));
		}
		else
		{
			MauCor::CoreServiceLocator::RegisterLogger(MauCor::CreateConsoleLogger());
		}

		if constexpr (ENABLE_DEBUG_RENDERING)
		{
			ServiceLocator::RegisterDebugRenderer(MauRen::CreateDebugRenderer(false));
		}

		InternalServiceLocator::RegisterRenderer(MauRen::CreateVulkanRenderer(m_Window->window, DEBUG_RENDERER));
		InternalServiceLocator::GetRenderer().Init();

		m_Window->InitWindowEvent_PostRendererInit();
		SDL_GL_SetSwapInterval(0);

		if constexpr (USE_IMGUI)
		{
			InternalServiceLocator::RegisterGUILayer(std::move(std::make_unique<ImGUILayer>()));
			InternalServiceLocator::GetGUILayer().Init(m_Window.get());

			MauCor::CoreServiceLocator::RegisterLogger(std::move(std::make_unique<MauEng::ImGUILogger>()));
		}

		ServiceLocator::RegisterSoundSystem(std::move(MAudio::CreateWWiseSoundSystem()));
		ME_ENGINE_CHECK(SOUND_SYSTEM.Initialize());

		// Also initializes input manager
		auto& inputManager{ InputManager::GetInstance() };
		inputManager.Init();

		if constexpr(ENABLE_PROFILER)
		{
			inputManager.BindAction("PROFILE", KeyInfo{ SDLK_F1, KeyInfo::ActionType::Down });
		}
	}

	Engine::~Engine()
	{
		SCENE_MANAGER.Destroy();

		ME_ENGINE_CHECK(SOUND_SYSTEM.Destroy());

		auto& inputManager{ InputManager::GetInstance() };
		inputManager.Destroy();

		InternalServiceLocator::GetGUILayer().Destroy();

		// Cleanup all core dependences & singletons
		InternalServiceLocator::GetRenderer().Destroy();

		m_Window->Destroy();
	}

	void Engine::Run(std::function<void()> const& load) noexcept
	{
		ME_PROFILE_BEGIN_SESSION("Startup", "Profiling/Startup/Startup")
		// First load everything the user wants us to load using their "load function"
		load();
		ME_PROFILE_END_SESSION()

		GameLoop();
	}

	void Engine::GameLoop() noexcept
	{
		using namespace MauRen;

		// FPS
		int frameCount{ 0 };
		float elapsedTime{ 0.f };

		bool IsMinimised = false;

		// Get all the systems we wish to use during the game loop
		auto& time{ TIME };
		auto& sceneManager{ SceneManager::GetInstance() };
		auto& inputManager{ InputManager::GetInstance() };
		auto& eventManager{ MauCor::EventManager::GetInstance() };
		bool doContinue{ true };

		while (doContinue)
		{
			if constexpr(ENABLE_PROFILER)
			{
				if (inputManager.IsActionExecuted("PROFILE"))
				{
					PROFILER.Start("Profiling/Run/Run");
				}

				ME_PROFILE_FRAME()
			}

			SDL_GetWindowFlags(m_Window->window) & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN) ? IsMinimised = true : IsMinimised = false;

			time.Update();

			if (LOG_FPS)
			{
				if (IsMinimised)
				{
					LOGGER.Log(MauCor::ELogPriority::Info, LogEngine, "Window is minimized");

					elapsedTime = 0.f;
					frameCount = 0;
				}

				elapsedTime += time.ElapsedSec();
				++frameCount;

				// Log FPS every second
				if (elapsedTime >= 1.0f)
				{
					float const fps{ static_cast<float>(frameCount) / elapsedTime };
					LOGGER.Log(MauCor::ELogPriority::Info, LogEngine, "FPS: {}", fps);
					elapsedTime -= 1.0f;
					frameCount = 0;
				}
			}

			doContinue = inputManager.ProcessInput(m_Window->window);
			eventManager.ProcessEvents();

			while (time.IsLag())
			{
				if (not IsMinimised)
				{
					sceneManager.FixedUpdate();
				}

				time.ProcessLag();
			}

			if (not IsMinimised)
			{
				sceneManager.Tick();

			#pragma region IMGUI
				InternalServiceLocator::GetGUILayer().BeginFrame();
				InternalServiceLocator::GetGUILayer().Render(sceneManager.GetActiveScene(), m_Window.get());
				InternalServiceLocator::GetGUILayer().EndFrame();
			#pragma endregion

				sceneManager.Render({m_Window->width, m_Window->height});
			}

			if constexpr (LIMIT_FPS)
			{
				ME_PROFILE_SCOPE("Main Thread Sleep")
				std::this_thread::sleep_for(time.SleepTime());
			}

			PROFILER.Update();
		}
	}
}
