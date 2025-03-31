#include "Engine.h"

#include <algorithm>
#include <thread>

#include "Camera.h"
#include "Renderer.h"
#include "RendererFactory.h"

#include "Core/GLFWWindow.h"

#include "glm/glm.hpp"


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

		Camera m_Camera{ glm::vec3{0.f, -8.f, 3.f }, 60.f, static_cast<float>(m_Window->width / m_Window->height) };
		m_Camera.Focus({ 0,0,0 });
		// Get all the systems we wish to use during the game loop

		Mesh m1{ "Models/Gun.obj" };
		Mesh m2{ "Models/Skull.obj" };

		m_Renderer->UpLoadModel(m1);
		m_Renderer->UpLoadModel(m2);

		MeshInstance mi1{ m2 };
		mi1.Translate({ 5, 20,  -3 });
		mi1.Scale({ .3f, .3f, .3f });

		MeshInstance mi2{ m2 };
		mi2.Translate({ -5, 20,  -8 });
		mi2.Scale({ .3f, .3f, .3f });

		MeshInstance mi3{ m1 };
		mi3.Translate({ 0, 0,  0 });
		mi3.Rotate(glm::radians(90.f), {1, 0,  0});
		mi3.Scale({ 5.f, 5.f, 5.f });

		// TODO
		// The Game loop
		bool doContinue{ true };
		while (doContinue)
		{
			static auto startTime{ std::chrono::high_resolution_clock::now() };

			auto const currentTime{ std::chrono::high_resolution_clock::now() };
			float const deltaTime{ std::chrono::duration<float>(currentTime - startTime).count() };

			startTime = currentTime; // Update start time for the next frame


			// TODO setup time class
			//time.Update();


			glfwPollEvents();
			if (glfwWindowShouldClose(m_Window->window))
			{
				doContinue = false;
			}

			// TODO setup input class
			//doContinue = input.ProcessInput();

			if (glfwGetKey(m_Window->window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				m_Camera.Translate({ 0.f, 0.f, 4*deltaTime });
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				m_Camera.Translate({ 0.f, 0.f, -4.f* deltaTime });
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				m_Camera.Translate({ -4.f* deltaTime, 0.f, 0.f });
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				m_Camera.Translate({ 4.f* deltaTime, 0.f, 0.f });
			}

			constexpr float rotSpeed{ 4 };
			if (glfwGetKey(m_Window->window, GLFW_KEY_I) == GLFW_PRESS)
			{
				float rot = rotSpeed * deltaTime;
				m_Camera.Rotate(rot, {1,0,0});
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_K) == GLFW_PRESS)
			{
				float rot = rotSpeed * deltaTime;
				m_Camera.Rotate(rot, { -1,0,0 });
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_J) == GLFW_PRESS)
			{
				float rot = rotSpeed * deltaTime * 3;
				m_Camera.Rotate(rot, {0,0,1});
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_L) == GLFW_PRESS)
			{
				float rot = rotSpeed * deltaTime * 3;
				m_Camera.Rotate(rot, { 0,0,-1 });
			}

			//while (time.IsLag())
			//{
			//	sceneManager.FixedUpdate();
			//	time.ProcessLag();
			//}

			m_Camera.Update();
			//sceneManager.Update();

			float rotationSpeed = glm::radians(90.0f); // 90 degrees per second
			mi1.Rotate(rotationSpeed * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f));
			mi2.Rotate(rotationSpeed * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f));
			mi3.Rotate(rotationSpeed * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));

			mi1.Draw();
			mi2.Draw();
			mi3.Draw();

			m_Renderer->Render(m_Camera.GetViewMatrix(), m_Camera.GetProjectionMatrix());

		//	std::this_thread::sleep_for();
		}
	}
}
