#include "PCH.h"

#include "GameScene.h"
#include <iostream>

#include "GameTime.h"
#include <glm/glm.hpp>

#include <random>  // For random number generation

namespace MauGam
{
	GameScene::GameScene()
	{
		ME_PROFILE_FUNCTION()

		m_CameraManager.GetActiveCamera().SetPosition(glm::vec3{ 0.f, 2, 4 });
		m_CameraManager.GetActiveCamera().SetFOV(60.f);

		m_CameraManager.GetActiveCamera().Focus({ 0,0,1 });

		using namespace MauRen;
		// init meshes
		Mesh m1{ "Resources/Models/Gun.obj" };
		Mesh m2{ "Resources/Models/Skull.obj" };

		RENDERER.UpLoadModel(m1);
		RENDERER.UpLoadModel(m2);

		// Skulls
		MeshInstance mi1{ m2 };
		mi1.Translate({ 5, 5,  -20 });
		mi1.Scale({ .3f, .3f, .3f });
		mi1.Rotate({ 270, 0, 0 });

		MeshInstance mi2{ m2 };
		mi2.Translate({ -5, 5,  -20 });
		mi2.Scale({ .3f, .3f, .3f });
		mi2.Rotate({ 270, 0, 0 });

		// Gun
		MeshInstance mi3{ m1 };
		mi3.Translate({ 0, 2,  0 });
		mi3.Scale({ 5.f, 5.f, 5.f });

		std::random_device rd;  // Random device for seed
		std::mt19937 gen(rd()); // Mersenne Twister generator
		std::uniform_real_distribution<float> dis(-20.0f, 20.0f); // Random translation range

		m_Mehses.emplace_back(mi1);
		m_Mehses.emplace_back(mi2);
		for (size_t i = 0; i < 99'000; i++)
		{
			m_Mehses.emplace_back(mi3);

			m_Mehses.back().Translate({ dis(gen), dis(gen), dis(gen) });
		}
		m_Mehses.emplace_back(mi3);


		// Setup input
		auto& input{ INPUT_MANAGER };
		input.BindAction("MoveUp", MauEng::KeyInfo{SDLK_UP, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveLeft", MauEng::KeyInfo{ SDLK_LEFT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveRight", MauEng::KeyInfo{ SDLK_RIGHT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveDown", MauEng::KeyInfo{ SDLK_DOWN, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("RotUp", MauEng::KeyInfo{ SDLK_I, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotLeft", MauEng::KeyInfo{ SDLK_J, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotRight", MauEng::KeyInfo{ SDLK_L, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotDown", MauEng::KeyInfo{ SDLK_K, MauEng::KeyInfo::ActionType::Held });


		input.BindAction("Rotate", MauEng::MouseInfo{ {},   MauEng::MouseInfo::ActionType::Moved });

		auto& w = GetECSWorld();
		MauEng::Entity e = w.CreateEntity();
	}

	void GameScene::OnLoad()
	{
		ME_PROFILE_FUNCTION()

		Scene::OnLoad();
		
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "Demo Scene Loaded! ");
	}

	void GameScene::Tick()
	{
		ME_PROFILE_FUNCTION()

		Scene::Tick();

		auto const& input{ INPUT_MANAGER };

		auto constexpr movementSpeed{ 20.f };
		if (input.IsActionExecuted("MoveUp"))
		{
			m_CameraManager.GetActiveCamera().Translate({ 0.f, 0.f, movementSpeed * TIME.ElapsedSec() });
		}
		if (input.IsActionExecuted("MoveDown"))
		{
			m_CameraManager.GetActiveCamera().Translate({ 0.f, 0.f, -movementSpeed * TIME.ElapsedSec() });
		}
		if (input.IsActionExecuted("MoveLeft"))
		{
			m_CameraManager.GetActiveCamera().Translate({ -movementSpeed * TIME.ElapsedSec(), 0.f, 0.f });
		}
		if (input.IsActionExecuted("MoveRight"))
		{
			m_CameraManager.GetActiveCamera().Translate({ movementSpeed * TIME.ElapsedSec(), 0.f, 0.f });
		}

		float constexpr keyboardRotSpeed{ 10 };
		if (input.IsActionExecuted("RotLeft"))
		{
			float const rot{ -keyboardRotSpeed * TIME.ElapsedSec() * 3 };
			m_CameraManager.GetActiveCamera().RotateX(rot);
		}
		if (input.IsActionExecuted("RotRight"))
		{
			float const rot{ keyboardRotSpeed * TIME.ElapsedSec() * 3 };
			m_CameraManager.GetActiveCamera().RotateX(rot);
		}
		if (input.IsActionExecuted("RotUp"))
		{
			float const rot{ keyboardRotSpeed * TIME.ElapsedSec() };
			m_CameraManager.GetActiveCamera().RotateY(rot);
		}
		if (input.IsActionExecuted("RotDown"))
		{
			float const rot{ -keyboardRotSpeed * TIME.ElapsedSec() };
			m_CameraManager.GetActiveCamera().RotateY(rot);
		}

		float constexpr mouseRotSpeed{ 60 };
		if (input.IsActionExecuted("Rotate"))
		{
			auto const mouseMovement{ input.GetDeltaMouseMovement() };
			float const rot{ mouseRotSpeed * TIME.ElapsedSec() };

			m_CameraManager.GetActiveCamera().RotateX(mouseMovement.first * rot);
			m_CameraManager.GetActiveCamera().RotateY(-mouseMovement.second * rot);
		}

		// 90 degrees per second
		float constexpr rotationSpeed{ 90.0f };
		m_Mehses[0].Rotate({ 0, 0, rotationSpeed * TIME.ElapsedSec() });
		m_Mehses[1].Rotate({ 0, 0, -rotationSpeed * TIME.ElapsedSec() });

		for (size_t i{ 2 }; i < m_Mehses.size(); ++i)
		{
			m_Mehses[i].Rotate({ 0, rotationSpeed * TIME.ElapsedSec() });
		}
	}

	void GameScene::OnRender() const
	{
		Scene::OnRender();
		{
			ME_PROFILE_SCOPE("Queue draws");
			for (auto const& m : m_Mehses)
			{
				m.Draw();
			}
		}

	}
}
