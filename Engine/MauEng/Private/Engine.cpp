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

#include "backends/imgui_impl_sdl3.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

namespace MauEng
{
	Engine::Engine():
		m_Window{ std::make_unique<SDLWindow>() }
	{
		// Initialize all core dependences & singletons
		if constexpr (ENABLE_FILE_LOGGING)
		{
			MauCor::CoreServiceLocator::RegisterLogger(MauCor::CreateFileLogger("Log.txt"));
			MauCor::CoreServiceLocator::GetLogger().SetPriorityLevel(MauCor::ELogPriority::Warn);
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

#pragma region INIT_IMGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io{ ImGui::GetIO() };
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.DisplaySize = ImVec2{ static_cast<float>(m_Window->width), static_cast<float>(m_Window->height) };

		InternalServiceLocator::GetRenderer().InitImGUI();
#pragma endregion

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
		auto& inputManager{ InputManager::GetInstance() };
		inputManager.Destroy();

		// Cleanup all core dependences & singletons
		InternalServiceLocator::GetRenderer().Destroy();

		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

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

				RENDERER.BeginImGUIFrame();

				// Create main dockspace window
				ImGuiWindowFlags windowFlags{ ImGuiWindowFlags_NoDocking };
				windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
				windowFlags |= ImGuiWindowFlags_NoBackground;

				ImGuiViewport* viewport{ ImGui::GetMainViewport() };
				ImGui::SetNextWindowPos(viewport->WorkPos);
				ImGui::SetNextWindowSize(viewport->WorkSize);
				ImGui::SetNextWindowViewport(viewport->ID);

				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

				ImGui::Begin("DockSpaceBackground", nullptr, windowFlags);
				ImGui::PopStyleVar(2);

				// Use flags to prevent window splitting in center
				ImGuiID const dockspaceID{ ImGui::GetID("MyDockSpace") };
				ImGui::DockSpace(dockspaceID, ImVec2{ 0.0f, 0.0f }, ImGuiDockNodeFlags_PassthruCentralNode);

				ImGui::End();

				// Temp test
				ImGui::Begin("Debug Info");
				ImGui::Text("Frame time: %.3f ms", 10.f);
				ImGui::End();

				RENDERER.EndImGUIFrame();

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
