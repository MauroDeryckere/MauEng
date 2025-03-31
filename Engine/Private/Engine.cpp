#include "Engine.h"

#include <algorithm>
#include <thread>

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

		Mesh m1{ "Models/Gun.obj" };
		Mesh m2{ "Models/Skull.obj" };

		m_Renderer->UpLoadModel(m1);
		m_Renderer->UpLoadModel(m2);

		MeshInstance mi1{ m2 };
		mi1.Translate({ 0, 3,  -3 });
		mi1.Scale({ .3f, .3f, .3f });

		MeshInstance mi2{ m2 };
		mi2.Translate({ 0, 3,  -8 });
		mi2.Scale({ .3f, .3f, .3f });

		MeshInstance mi3{ m1 };
		mi3.Translate({ 0, -1,  -3 });
		mi3.Scale({ 5.f, 5.f, 5.f });

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

			static auto startTime{ std::chrono::high_resolution_clock::now() };

			auto const currentTime{ std::chrono::high_resolution_clock::now() };
			float const deltaTime{ std::chrono::duration<float>(currentTime - startTime).count() };
			startTime = currentTime; // Update start time for the next frame

			float rotationSpeed = glm::radians(90.0f); // 90 degrees per second
			mi1.Rotate(rotationSpeed * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f));
			mi2.Rotate(rotationSpeed * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f));
			mi3.Rotate(rotationSpeed * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f));

			mi1.Draw();
			mi2.Draw();
			mi3.Draw();

			m_Renderer->Render();

		//	std::this_thread::sleep_for();
		}
	}
}
