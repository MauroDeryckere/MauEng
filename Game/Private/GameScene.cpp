#include "GameScene.h"

#include "ServiceLocator.h"

#include <iostream>

#include "GameTime.h"
#include <glm/glm.hpp>

namespace MauGam
{
	GameScene::GameScene()
	{
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
		//TODO rotate skulls to correct dir
		MeshInstance mi1{ m2 };
		mi1.Translate({ 5, 20,  -3 });
		mi1.Scale({ .3f, .3f, .3f });

		MeshInstance mi2{ m2 };
		mi2.Translate({ -5, 20,  -8 });
		mi2.Scale({ .3f, .3f, .3f });

		// Gun
		MeshInstance mi3{ m1 };
		mi3.Translate({ 0, 2,  0 });
		mi3.Scale({ 5.f, 5.f, 5.f });

		m_Mehses.emplace_back(mi1);
		m_Mehses.emplace_back(mi2);
		m_Mehses.emplace_back(mi3);

		// Setup input
		auto& input{ MauEng::InputManager() };
		input.BindAction("MoveUp", MauEng::KeyInfo{SDLK_UP, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveLeft", MauEng::KeyInfo{ SDLK_LEFT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveRight", MauEng::KeyInfo{ SDLK_RIGHT, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("MoveDown", MauEng::KeyInfo{ SDLK_DOWN, MauEng::KeyInfo::ActionType::Held });

		input.BindAction("RotUp", MauEng::KeyInfo{ SDLK_I, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotLeft", MauEng::KeyInfo{ SDLK_J, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotRight", MauEng::KeyInfo{ SDLK_L, MauEng::KeyInfo::ActionType::Held });
		input.BindAction("RotDown", MauEng::KeyInfo{ SDLK_K, MauEng::KeyInfo::ActionType::Held });


		input.BindAction("Rotate", MauEng::MouseInfo{ {},   MauEng::MouseInfo::ActionType::Moved });
	}

	void GameScene::OnLoad()
	{
		Scene::OnLoad();
		
		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Game, "Demo Scene Loaded! ");
	}

	void GameScene::Tick()
	{
		Scene::Tick();

		// Demo logging tests
		//LOGGER.Log(MauCor::LogPriority::Error, MauCor::LogCategory::Game,"test {}", 1000);
		ME_LOG_ERROR(MauCor::LogCategory::Game, "TEST");

		// Demo debug drawing tests
		//DEBUG_RENDERER.DrawLine({0, 0,0 },  {0, 100, 100} );
		//DEBUG_RENDERER.DrawLine({-10 , 10, -10}, {10, 10, 10}, { 0, 1, 0});

		//DEBUG_RENDERER.DrawRect({ -10, 0, 0 }, { 10, 0, 0 }, { 10, 20, 5 }, { -10, 20, 5 }, {0, 0, 1});

		//DEBUG_RENDERER.DrawSphere({}, 20.f, { 1, 1, 0 });
		//DEBUG_RENDERER.DrawSphereComplex({ 20,20,20 }, 20.f, { 1, 1, 1 }, 24, 10);
		//DEBUG_RENDERER.DrawCube({}, 20);
		//DEBUG_RENDERER.DrawCube({}, 20, 50, 30);

		//DEBUG_RENDERER.DrawEllipse({}, 20, 50, { 1, 0, 0 });

		//DEBUG_RENDERER.DrawEllipsoid({}, 10, 20, 30.f, { 0, 1, 0 });
		//DEBUG_RENDERER.DrawEllipsoidComplex({}, 10, 20, 30.f, { 0, 0, 1 }, 24,6);

		//DEBUG_RENDERER.DrawArrow({}, { 2, 3, 3});
		//DEBUG_RENDERER.DrawCylinder({ -10, 0, -10 }, 10, 100);

		//DEBUG_RENDERER.DrawPolygon({ {0, 0, 0}, { 0, 19, 20 }, {32, 10, -10}, {10, 20, 5}, {-2, -2, -2 } });

		auto const& input{ MauEng::InputManager() };

		auto constexpr movementSpeed{ 20.f };
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

		float constexpr keyboardRotSpeed{ 10 };
		if (input.IsActionExecuted("RotLeft"))
		{
			float const rot{ -keyboardRotSpeed * MauEng::Time().ElapsedSec() * 3 };
			m_CameraManager.GetActiveCamera().RotateX(rot);
		}
		if (input.IsActionExecuted("RotRight"))
		{
			float const rot{ keyboardRotSpeed * MauEng::Time().ElapsedSec() * 3 };
			m_CameraManager.GetActiveCamera().RotateX(rot);
		}
		if (input.IsActionExecuted("RotUp"))
		{
			float const rot{ keyboardRotSpeed * MauEng::Time().ElapsedSec() };
			m_CameraManager.GetActiveCamera().RotateY(rot);
		}
		if (input.IsActionExecuted("RotDown"))
		{
			float const rot{ -keyboardRotSpeed * MauEng::Time().ElapsedSec() };
			m_CameraManager.GetActiveCamera().RotateY(rot);
		}

		float constexpr mouseRotSpeed{ 60 };
		if (input.IsActionExecuted("Rotate"))
		{
			auto const mouseMovement{ input.GetDeltaMouseMovement() };
			float const rot{ mouseRotSpeed * MauEng::Time().ElapsedSec() };

			m_CameraManager.GetActiveCamera().RotateX(mouseMovement.first * rot);
			m_CameraManager.GetActiveCamera().RotateY(-mouseMovement.second * rot);
		}

		// 90 degrees per second
	//	float constexpr rotationSpeed{ glm::radians(90.0f) };
	//	m_Mehses[0].Rotate(rotationSpeed * MauEng::Time().ElapsedSec(), glm::vec3(0.0f, 0.0f, 1.0f));
	//	m_Mehses[1].Rotate(rotationSpeed * MauEng::Time().ElapsedSec(), glm::vec3(0.0f, 0.0f, 1.0f));
	//	m_Mehses[2].Rotate(rotationSpeed * MauEng::Time().ElapsedSec(), glm::vec3(0.0f, 1.0f, 0.0f));
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
