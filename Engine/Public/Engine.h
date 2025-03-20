#ifndef MAUENG_ENGINE_H
#define MAUENG_ENGINE_H

#include <functional>

namespace MauEng
{
	class Engine
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
	};
}

#endif // MAUENG_ENGINE_H