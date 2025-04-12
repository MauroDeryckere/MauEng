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

#include "Logger/logger.h"

#include "../../Renderer/Private/DebugRenderer/InternalDebugRenderer.h"
#include "Input/KeyInfo.h"

namespace MauEng
{
	Engine::Engine():
		m_Window{ std::make_unique<SDLWindow>() }
	{
		// Initialize all core dependences & singletons

		if constexpr (ENABLE_FILE_LOGGING)
		{
			MauCor::CoreServiceLocator::RegisterLogger(MauCor::CreateFileLogger("Log.txt"));
			MauCor::CoreServiceLocator::GetLogger().SetPriorityLevel(MauCor::LogPriority::Warn);
		}
		else
		{
			MauCor::CoreServiceLocator::RegisterLogger(MauCor::CreateConsoleLogger());
		}

		if constexpr (ENABLE_DEBUG_RENDERING)
		{
			ServiceLocator::RegisterDebugRenderer(std::make_unique<MauRen::InternalDebugRenderer>());
		}

		ServiceLocator::RegisterRenderer(MauRen::CreateVulkanRenderer(m_Window->window, DEBUG_RENDERER));
		ServiceLocator::GetRenderer().Init();

		m_Window->Initialize();

		SDL_GL_SetSwapInterval(0);

		auto& inputManager{ InputManager::GetInstance() };

		if constexpr(ENABLE_PROFILER)
		{
			inputManager.BindAction("PROFILE", KeyInfo{ SDLK_F1, KeyInfo::ActionType::Down });
		}
	}

	Engine::~Engine()
	{
		// Cleanup all core dependences & singletons
		ServiceLocator::GetRenderer().Destroy();
	}

	void Engine::Run(std::function<void()> const& load)
	{
		ME_PROFILE_BEGIN_SESSION("Startup", "Profiling/Startup/Startup")
		// First load everything the user wants us to load using their "load function"
		load();
		ME_PROFILE_END_SESSION()

		GameLoop();
	}

	void Engine::GameLoop()
	{
		using namespace MauRen;

		// FPS
		int frameCount{ 0 };
		float elapsedTime{ 0.f };

		bool IsMinimised = false;

		// Get all the systems we wish to use during the game loop
		auto& time{ Time::GetInstance() };
		auto& sceneManager{ SceneManager::GetInstance() };
		auto& inputManager{ InputManager::GetInstance() };

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

			if constexpr (LOG_FPS)
			{
				if (IsMinimised)
				{
					LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Engine, "Window is minimized");

					elapsedTime = 0.f;
					frameCount = 0;
				}

				elapsedTime += time.ElapsedSec();
				++frameCount;

				// Log FPS every second
				if (elapsedTime >= 1.0f)
				{
					float const fps{ static_cast<float>(frameCount) / elapsedTime };
					LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Engine, "FPS: {}", fps);
					elapsedTime -= 1.0f;
					frameCount = 0;
				}
			}

			doContinue = inputManager.ProcessInput();

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
				sceneManager.Render();
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
