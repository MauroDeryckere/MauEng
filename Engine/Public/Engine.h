#ifndef MAUENG_ENGINE_H
#define MAUENG_ENGINE_H

#include <functional>
#include <memory>

namespace MauRen
{
	class Renderer;
}

namespace MauEng
{
	struct GLFWWindow;

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
		std::unique_ptr<GLFWWindow> m_Window{ nullptr };
		std::unique_ptr<MauRen::Renderer> m_Renderer{ nullptr };
	};
}

#endif // MAUENG_ENGINE_H