#ifndef MAUREN_DEBUGRENDERER_H
#define MAUREN_DEBUGRENDERER_H

#include "DebugVertex.h"

namespace MauRen
{
	class Renderer;

	class DebugRenderer final
	{
	public:
		explicit DebugRenderer() = default;
		~DebugRenderer() = default;

		void DrawLine(DebugVertex const& start, DebugVertex const& end);

		DebugRenderer(DebugRenderer const&) = delete;
		DebugRenderer(DebugRenderer&&) = delete;
		DebugRenderer& operator=(DebugRenderer const&) = delete;
		DebugRenderer& operator=(DebugRenderer&&) = delete;

	private:
		// Needs access to the variables during the render
		friend class VulkanRenderer;

		std::vector<DebugVertex> m_ActiveLines{};
		uint32_t const MAX_LINES{ 1'000 };
	};
}

#endif // MAUREN_DEBUGRENDERER_H