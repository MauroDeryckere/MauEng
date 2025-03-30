#include "Engine.h"

#include <algorithm>
#include "Renderer.h"
#include "RendererFactory.h"

#include "Core/GLFWWindow.h"

namespace MauEng
{
	Engine::Engine():
		m_Window{ std::make_unique<GLFWWindow>() },
		m_Renderer{ MauRen::CreateVulkanRenderer(m_Window->window) }
	{
		// Initialize all core dependences & singletons
		m_Window->Initialize(m_Renderer.get());
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

		Mesh m1{ "Models/VikingRoom.obj" };
		//Mesh m2{ "models/skull.obj" };

		m_Renderer->UpLoadModel(m1);
	//	m_Renderer->UpLoadModel(m2);

		MeshInstance mi1{ m1 };
		MeshInstance mi2{ m1 };
		MeshInstance mi3{ m1 };
		MeshInstance mi4{ m1 };
		MeshInstance mi5{ m1 };

		mi2.Translate({ 1, 0,0 });
		mi3.Translate({2, 0,0 });
		mi4.Translate({ 3, 0,0 });
		mi5.Translate({ 4, 0,0 });

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

			mi1.Draw();
			mi2.Draw();
			mi3.Draw();
			mi4.Draw();
			mi5.Draw();

			m_Renderer->Render();

			//std::this_thread::sleep_for(time.SleepTime());
		}
	}
}
