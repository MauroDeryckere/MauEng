#include "DebugRenderer.h"

namespace MauRen
{
	void DebugRenderer::DrawLine(DebugVertex const& start, DebugVertex const& end)
	{
		if (std::size(m_ActiveLines) < MAX_LINES)
		{
			m_ActiveLines.emplace_back(start);
			m_ActiveLines.emplace_back(end);
		}
	}
}
