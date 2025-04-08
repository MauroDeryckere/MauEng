#ifndef MAUENG_SERVICELOCATOR_H
#define MAUENG_SERVICELOCATOR_H

#include "Renderer.h"
#include "DebugRenderer.h"

#include "GameTime.h"
#include "Scene/SceneManager.h"
#include "Input/InputManager.h"

#include "Logger.h"

#include <memory>

namespace MauEng
{
	// Class used to access all engine services
	class ServiceLocator final
	{
	public:
		[[nodiscard]] static MauRen::Renderer& GetRenderer() { return (*m_pRenderer); }
		static void RegisterRenderer(std::unique_ptr<MauRen::Renderer>&& pRenderer)
		{
			m_pRenderer = ((!pRenderer) ? std::make_unique<MauRen::NullRenderer>(nullptr, *std::make_unique<MauRen::NullDebugRenderer>()) : std::move(pRenderer));
		}

		[[nodiscard]] static MauRen::DebugRenderer& GetDebugRenderer() { return (*m_pDebugRenderer); }
		static void RegisterDebugRenderer(std::unique_ptr<MauRen::DebugRenderer>&& pRenderer)
		{
			m_pDebugRenderer = ((!pRenderer) ? std::make_unique<MauRen::NullDebugRenderer>() : std::move(pRenderer));
		}

		[[nodiscard]] static MauEng::Time& GetTime() { return MauEng::Time::GetInstance(); }
		[[nodiscard]] static MauEng::SceneManager& GetSceneManager() { return MauEng::SceneManager::GetInstance(); }
		[[nodiscard]] static MauEng::InputManager& GetInputManager() { return MauEng::InputManager::GetInstance(); }

		[[nodiscard]] static MauCor::Logger& GetLogger() { return (*m_pLogger); }
		static void RegisterLogger(std::unique_ptr<MauCor::Logger>&& pLogger)
		{
			m_pLogger = ((!pLogger) ? std::make_unique<MauCor::NullLogger>() : std::move(pLogger));
		}

	private:
		static std::unique_ptr<MauRen::Renderer> m_pRenderer;
		static std::unique_ptr<MauRen::DebugRenderer> m_pDebugRenderer;

		static std::unique_ptr<MauCor::Logger> m_pLogger;
	};

#pragma region EasyAccessHelper
	// Helper function for easy access to the renderer
	inline MauRen::Renderer& Renderer()
	{
		return ServiceLocator::GetRenderer();
	}

	// Helper function for easy access to the debug renderer
	inline MauRen::DebugRenderer& DebugRenderer()
	{
		return ServiceLocator::GetDebugRenderer();
	}

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

	// Helper function for easy access to the logger
	inline MauCor::Logger& Logger()
	{
		return ServiceLocator::GetLogger();
	}

	// Macros
	#define RENDERER MauEng::ServiceLocator::GetRenderer()
	#define DEBUG_RENDERER MauEng::ServiceLocator::GetDebugRenderer()

	#define TIME MauEng::ServiceLocator::GetTime()
	#define SCENE_MANAGER MauEng::ServiceLocator::GetSceneManager()
	#define INPUT_MANAGER MauEng::ServiceLocator::GetInputManager()

	#define LOGGER MauEng::ServiceLocator::GetLogger()

	#define ME_LOG(priority, category, fmtStr, ...) \
			LOGGER.Log(priority, category, fmtStr, __VA_ARGS__)

	#define ME_LOG_TRACE(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Trace, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_INFO(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Info, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_DEBUG(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Debug, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_WARN(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Warn, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_ERROR(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Error, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_FATAL(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Fatal, category, fmtStr, __VA_ARGS__)

#pragma endregion
}

#endif