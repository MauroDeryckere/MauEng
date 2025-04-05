#include "GameScene.h"

#include "ServiceLocator.h"

#include <iostream>

#include "GameTime.h"
#include <glm/glm.hpp>

namespace MauGam
{
	GameScene::GameScene()
	{
		m_CameraManager.GetActiveCamera().SetPosition(glm::vec3{ 0.f, -8.f, 3.f });
		m_CameraManager.GetActiveCamera().SetFOV(60.f);

		m_CameraManager.GetActiveCamera().Focus({ 0,0,0 });

		using namespace MauRen;
		// init meshes
		Mesh m1{ "Models/Gun.obj" };
		Mesh m2{ "Models/Skull.obj" };

		MauEng::Renderer().UpLoadModel(m1);
		MauEng::Renderer().UpLoadModel(m2);

		MeshInstance mi1{ m2 };
		mi1.Translate({ 5, 20,  -3 });
		mi1.Scale({ .3f, .3f, .3f });

		MeshInstance mi2{ m2 };
		mi2.Translate({ -5, 20,  -8 });
		mi2.Scale({ .3f, .3f, .3f });

		MeshInstance mi3{ m1 };
		mi3.Translate({ 0, 0,  0 });
		mi3.Rotate(glm::radians(90.f), { 1, 0,  0 });
		mi3.Scale({ 5.f, 5.f, 5.f });

		m_Mehses.emplace_back(mi1);
		m_Mehses.emplace_back(mi2);
		m_Mehses.emplace_back(mi3);

		auto& input{ MauEng::InputManager() };

		input.BindAction("MoveUp", {SDLK_UP, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveLeft", { SDLK_LEFT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveRight", { SDLK_RIGHT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveDown", { SDLK_DOWN, MauEng::KeyInfo::ActionType::Held });
	}

	void GameScene::OnLoad()
	{
		Scene::OnLoad();

		std::cout << "Demo Scene loaded! \n";
	}

	void GameScene::Tick()
	{
		Scene::Tick();

		auto const movementSpeed{ 20.f };

		auto const& input{ MauEng::InputManager() };
		if (input.IsActionExecuted("MoveUp"))
		{
			m_CameraManager.GetActiveCamera().Translate({ 0.f, 0.f, movementSpeed * MauEng::Time().ElapsedSec() });
		}
		if (input.IsActionExecuted("MoveDown"))
		{
			m_CameraManager.GetActiveCamera().Translate({ 0.f, 0.f, -movementSpeed * MauEng::Time().ElapsedSec() });
		}
		if (input.IsActionExecuted("MoveLeft"))
		{
			m_CameraManager.GetActiveCamera().Translate({ -movementSpeed * MauEng::Time().ElapsedSec(), 0.f, 0.f });
		}
		if (input.IsActionExecuted("MoveRight"))
		{
			m_CameraManager.GetActiveCamera().Translate({ movementSpeed * MauEng::Time().ElapsedSec(), 0.f, 0.f });
		}

		//TODO mouse input
		//constexpr float rotSpeed{ 4 };
		//if (glfwGetKey(m_Window->window, GLFW_KEY_I) == GLFW_PRESS)
		//{
		//	float rot = rotSpeed * time.ElapsedSec();
		//	m_Camera.Rotate(rot, { 1,0,0 });
		//}
		//if (glfwGetKey(m_Window->window, GLFW_KEY_K) == GLFW_PRESS)
		//{
		//	float rot = rotSpeed * time.ElapsedSec();
		//	m_Camera.Rotate(rot, { -1,0,0 });
		//}
		//if (glfwGetKey(m_Window->window, GLFW_KEY_J) == GLFW_PRESS)
		//{
		//	float rot = rotSpeed * time.ElapsedSec() * 3;
		//	m_Camera.Rotate(rot, { 0,0,1 });
		//}
		//if (glfwGetKey(m_Window->window, GLFW_KEY_L) == GLFW_PRESS)
		//{
		//	float rot = rotSpeed * time.ElapsedSec() * 3;
		//	m_Camera.Rotate(rot, { 0,0,-1 });
		//}

		float const rotationSpeed{ glm::radians(90.0f) }; // 90 degrees per second
		m_Mehses[0].Rotate(rotationSpeed * MauEng::Time().ElapsedSec(), glm::vec3(0.0f, 0.0f, 1.0f));
		m_Mehses[1].Rotate(rotationSpeed * MauEng::Time().ElapsedSec(), glm::vec3(0.0f, 0.0f, 1.0f));
		m_Mehses[2].Rotate(rotationSpeed * MauEng::Time().ElapsedSec(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void GameScene::OnRender() const
	{
		Scene::OnRender();

		for (auto const& m : m_Mehses)
		{
			m.Draw();
		}
	}
}
