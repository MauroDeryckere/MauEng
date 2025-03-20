#include "Engine.h"

#include <algorithm>
#include "Renderer.h"
#include "RendererFactory.h"

#include "Core/GLFWWindow.h"

namespace MauEng
{
	Engine::Engine():
		m_Window{ std::make_unique<GLFWWindow>() }
	{
		// Initialize all core dependences & singletons
	}

	Engine::~Engine()
	{
		// Cleanup all core dependences & singletons
	}

	void Engine::Run(std::function<void()> const& load)
	{
		// First load everything the user wants us to load using their "load function"
		load();

		using namespace MauRen;
		// Get all the systems we wish to use during the game loop
		std::unique_ptr<Renderer> renderer { MauRen::CreateRenderer()};
		renderer->Initialize(m_Window->window);


		// TODO
		// The Game loop

		bool doContinue{ true };
		while (doContinue)
		{
			// TODO setup time class
			//time.Update();


			glfwPollEvents();
			if (glfwWindowShouldClose(m_Window->window))
			{
				doContinue = false;
			}

			// TODO setup input class
			//doContinue = input.ProcessInput();

			//while (time.IsLag())
			//{
			//	sceneManager.FixedUpdate();
			//	time.ProcessLag();
			//}

			//sceneManager.Update();

			renderer->Render();

			//std::this_thread::sleep_for(time.SleepTime());
		}

	}
}
