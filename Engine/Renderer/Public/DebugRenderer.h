#ifndef MAUREN_DEBUGRENDERER_H
#define MAUREN_DEBUGRENDERER_H

#include "DebugVertex.h"

namespace MauRen
{
	class Renderer;

	class DebugRenderer final
	{
	public:
		explicit DebugRenderer() {}
		virtual ~DebugRenderer() = default;

		void Render();

		void DrawLine(DebugVertex const& start, DebugVertex const& end);

		DebugRenderer(DebugRenderer const&) = delete;
		DebugRenderer(DebugRenderer&&) = delete;
		DebugRenderer& operator=(DebugRenderer const&) = delete;
		DebugRenderer& operator=(DebugRenderer&&) = delete;

	private:
		std::vector<std::pair<DebugVertex, DebugVertex>> m_ActiveLines;
		uint32_t const MAX_LINES{ 1'000 };
	};

	inline void DebugRenderer::DrawLine(DebugVertex const& start, DebugVertex const& end)
	{
		if (std::size(m_ActiveLines) < MAX_LINES)
		{
			m_ActiveLines.emplace_back(start, end);
		}
	}
}

#endif // MAUREN_DEBUGRENDERER_H