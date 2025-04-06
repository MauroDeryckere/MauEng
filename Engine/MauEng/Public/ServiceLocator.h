#ifndef MAUENG_SERVICELOCATOR_H
#define MAUENG_SERVICELOCATOR_H

#include "Renderer.h"
#include "DebugRenderer.h"
#include "GameTime.h"
#include "Scene/SceneManager.h"
#include "Input/InputManager.h"

#include <memory>

namespace MauEng
{
	class ServiceLocator final
	{
	public:
		[[nodiscard]] static MauRen::Renderer& GetRenderer() { return (*m_pRenderer); }
		static void RegisterRenderer(std::unique_ptr<MauRen::Renderer>&& pRenderer)
		{
			m_pRenderer = ((!pRenderer) ? std::make_unique<MauRen::NullRenderer>(nullptr, *std::make_unique<MauRen::DebugRenderer>()) : std::move(pRenderer));
		}

		[[nodiscard]] static MauRen::DebugRenderer& GetDebugRenderer() { return (*m_pDebugRenderer); }
		static void RegisterDebugRenderer(std::unique_ptr<MauRen::DebugRenderer>&& pRenderer)
		{
			m_pDebugRenderer = ((!pRenderer) ? std::make_unique<MauRen::DebugRenderer>() : std::move(pRenderer));
		}


		[[nodiscard]] static MauEng::Time& GetTime() { return MauEng::Time::GetInstance(); }
		[[nodiscard]] static MauEng::SceneManager& GetSceneManager() { return MauEng::SceneManager::GetInstance(); }
		[[nodiscard]] static MauEng::InputManager& GetInputManager() { return MauEng::InputManager::GetInstance(); }

	private:
		static std::unique_ptr<MauRen::Renderer> m_pRenderer;
		static std::unique_ptr<MauRen::DebugRenderer> m_pDebugRenderer;
	};

#pragma region EasyAccessHelper

#define RENDERER ServiceLocator::GetRenderer()
#define DEBUG_RENDERER ServiceLocator::GetDebugRenderer()

	// Helper function for easy access to the time
	inline MauEng::Time& Time()
	{
		return ServiceLocator::GetTime();
	}

	// Helper function for easy access to the scene manager
	inline MauEng::SceneManager& SceneManager()
	{
		return ServiceLocator::GetSceneManager();
	}

	// Helper function for easy access to the input manager
	inline MauEng::InputManager& InputManager()
	{
		return ServiceLocator::GetInputManager();
	}
#pragma endregion
}

#endif