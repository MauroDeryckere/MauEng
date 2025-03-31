#include "Engine.h"

#include <algorithm>
#include <thread>

#include "Camera.h"
#include "RendererFactory.h"
#include "Scene/SceneManager.h"
#include "Renderer.h"

#include "Core/GLFWWindow.h"

#include "glm/glm.hpp"


namespace MauEng
{
	Engine::Engine():
		m_Window{ std::make_unique<GLFWWindow>() }
	{
		// Initialize all core dependences & singletons

		ServiceLocator::RegisterRenderer(MauRen::CreateVulkanRenderer(m_Window->window));
		ServiceLocator::GetRenderer().Init();
		m_Window->Initialize();
	}

	Engine::~Engine()
	{
		// Cleanup all core dependences & singletons
		ServiceLocator::GetRenderer().Destroy();
	}

	void Engine::Run(std::function<void()> const& load)
	{
		bool constexpr LIMIT_FPS{ false };

		// FPS tracking 
		bool constexpr LOG_FPS{ true };
		int frameCount{ 0 };
		float elapsedTime{ 0.f };

		// First load everything the user wants us to load using their "load function"
		load();
		
		using namespace MauRen;

		// Get all the systems we wish to use during the game loop
		auto& time{ Time::GetInstance() };
		auto& sceneManager{ SceneManager::GetInstance() };

		bool doContinue{ true };
		while (doContinue)
		{
			time.Update();

			if constexpr(LOG_FPS)
			{
				elapsedTime += time.ElapsedSec();
				++frameCount;

				// Log FPS every second
				if (elapsedTime >= 1.0f)
				{
					float const fps{ static_cast<float>(frameCount) / elapsedTime };
					std::cout << "FPS: " << fps << "\n";
					elapsedTime -= 1.0f;
					frameCount = 0;
				}
			}

			glfwPollEvents();
			if (glfwWindowShouldClose(m_Window->window))
			{
				doContinue = false;
			}

			// TODO setup input class
			//doContinue = input.ProcessInput();

			while (time.IsLag())
			{
				sceneManager.FixedUpdate();
				time.ProcessLag();
			}

			sceneManager.Tick();

			sceneManager.Render();

			if constexpr (LIMIT_FPS)
			{
				std::this_thread::sleep_for(time.SleepTime());
			}
		}
	}
}
