#include "Engine.h"

#include "Renderer.h"


namespace MauEng
{
	Engine::Engine()
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

		// Get all the systems we wish to use during the game loop
		MauRen::Renderer renderer{};

		// TODO refactor renderer
		renderer.RenderRun();

		// TODO
		// The Game loop

		bool doContinue{ true };
		while (doContinue)
		{
			// TODO setup time class
			//time.Update();

			// TODO setup input class
			//doContinue = input.ProcessInput();

			//while (time.IsLag())
			//{
			//	sceneManager.FixedUpdate();
			//	time.ProcessLag();
			//}

			//sceneManager.Update();
			//renderer.Render();

			//std::this_thread::sleep_for(time.SleepTime());
		}

	}
}
