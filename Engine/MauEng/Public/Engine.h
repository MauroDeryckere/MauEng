#ifndef MAUENG_ENGINE_H
#define MAUENG_ENGINE_H

#include "PCH.h"

#include <functional>
#include <memory>

namespace MauEng
{
	struct GLFWWindow;
	struct SDLWindow;

	class Engine final
	{
	public:
		explicit Engine();
		~Engine();

		void Run(std::function<void()> const& load);

		Engine(Engine const&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(Engine const&) = delete;
		Engine& operator=(Engine&&) = delete;

	private:
		std::unique_ptr<SDLWindow> m_Window;

		void GameLoop();
	};
}

#endif // MAUENG_ENGINE_H