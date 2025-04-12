#ifndef MAUENG_SERVICELOCATOR_H
#define MAUENG_SERVICELOCATOR_H

#include "Renderer.h"
#include "DebugRenderer.h"

#include "GameTime.h"
#include "Scene/SceneManager.h"
#include "Input/InputManager.h"

#include "CoreServiceLocator.h"

#include <memory>

namespace MauEng
{
	// Class used to access all engine services
	class ServiceLocator final
	{
	public:
		[[nodiscard]] static MauRen::Renderer& GetRenderer() { return (*m_pRenderer); }
		static void RegisterRenderer(std::unique_ptr<MauRen::Renderer>&& pRenderer);

		[[nodiscard]] static MauRen::DebugRenderer& GetDebugRenderer() { return (*m_pDebugRenderer); }
		static void RegisterDebugRenderer(std::unique_ptr<MauRen::DebugRenderer>&& pRenderer);

		[[nodiscard]] static MauEng::Time& GetTime() { return MauEng::Time::GetInstance(); }
		[[nodiscard]] static MauEng::SceneManager& GetSceneManager() { return MauEng::SceneManager::GetInstance(); }
		[[nodiscard]] static MauEng::InputManager& GetInputManager() { return MauEng::InputManager::GetInstance(); }

	private:
		static std::unique_ptr<MauRen::Renderer> m_pRenderer;
		static std::unique_ptr<MauRen::DebugRenderer> m_pDebugRenderer;
	};

#pragma region EasyAccessHelper
	// Macros
	#define RENDERER MauEng::ServiceLocator::GetRenderer()
	#define DEBUG_RENDERER MauEng::ServiceLocator::GetDebugRenderer()

	#define TIME MauEng::ServiceLocator::GetTime()
	#define SCENE_MANAGER MauEng::ServiceLocator::GetSceneManager()
	#define INPUT_MANAGER MauEng::ServiceLocator::GetInputManager()

#pragma endregion
}

#endif