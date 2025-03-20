#include "Engine.h"

#include "Renderer.h"


namespace Engine
{
	void Engine::Run()
	{
		Renderer::Renderer renderer{};

		renderer.RenderRun();
	}
}
