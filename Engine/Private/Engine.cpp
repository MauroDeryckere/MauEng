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
		ServiceLocator::RegisterRenderer(MauRen::CreateVulkanRenderer(m_Window->window));
		ServiceLocator::GetRenderer().Init();
		// Initialize all core dependences & singletons
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

		// TODO move to scene setup
		Camera m_Camera{ glm::vec3{0.f, -8.f, 3.f }, 60.f, static_cast<float>(m_Window->width / m_Window->height) };
		m_Camera.Focus({ 0,0,0 });

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

			if (glfwGetKey(m_Window->window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				m_Camera.Translate({ 0.f, 0.f, 4 * time.ElapsedSec() });
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				m_Camera.Translate({ 0.f, 0.f, -4.f * time.ElapsedSec() });
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				m_Camera.Translate({ -4.f * time.ElapsedSec(), 0.f, 0.f });
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				m_Camera.Translate({ 4.f * time.ElapsedSec(), 0.f, 0.f });
			}

			constexpr float rotSpeed{ 4 };
			if (glfwGetKey(m_Window->window, GLFW_KEY_I) == GLFW_PRESS)
			{
				float rot = rotSpeed * time.ElapsedSec();
				m_Camera.Rotate(rot, {1,0,0});
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_K) == GLFW_PRESS)
			{
				float rot = rotSpeed * time.ElapsedSec();
				m_Camera.Rotate(rot, { -1,0,0 });
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_J) == GLFW_PRESS)
			{
				float rot = rotSpeed * time.ElapsedSec() * 3;
				m_Camera.Rotate(rot, {0,0,1});
			}
			if (glfwGetKey(m_Window->window, GLFW_KEY_L) == GLFW_PRESS)
			{
				float rot = rotSpeed * time.ElapsedSec() * 3;
				m_Camera.Rotate(rot, { 0,0,-1 });
			}

			while (time.IsLag())
			{
				sceneManager.FixedUpdate();
				time.ProcessLag();
			}

			m_Camera.Update();
			sceneManager.Tick();

			sceneManager.Render(m_Camera.GetViewMatrix(), m_Camera.GetProjectionMatrix());

			if constexpr (LIMIT_FPS)
			{
				std::this_thread::sleep_for(time.SleepTime());
			}
		}
	}
}
