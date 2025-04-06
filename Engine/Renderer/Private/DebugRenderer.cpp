#include "DebugRenderer.h"

namespace MauRen
{
	void DebugRenderer::DrawLine(glm::vec3 const& start, glm::vec3 const& end, glm::vec3 const& colour) noexcept
	{
		if (std::size(m_ActiveLines) + 2 < MAX_LINES)
		{
			m_IndexBuffer.emplace_back(m_ActiveLines.size());
			m_ActiveLines.emplace_back(start, colour);
			m_IndexBuffer.emplace_back(m_ActiveLines.size());
			m_ActiveLines.emplace_back(end, colour);
		}
	}

	void DebugRenderer::DrawLine(DebugVertex const& start, DebugVertex const& end) noexcept
	{
		if (std::size(m_ActiveLines) + 2 < MAX_LINES)
		{
			m_IndexBuffer.emplace_back(m_ActiveLines.size());
			m_ActiveLines.emplace_back(start);
			m_IndexBuffer.emplace_back(m_ActiveLines.size());
			m_ActiveLines.emplace_back(end);
		}
	}

	void DebugRenderer::DrawRect(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& p3, glm::vec3 const& colour) noexcept
	{
		if (std::size(m_ActiveLines) + 4 < MAX_LINES)
		{
			auto const idx01{ m_ActiveLines.size() };

			m_ActiveLines.emplace_back(p0, colour);
			m_ActiveLines.emplace_back(p1, colour);
			m_ActiveLines.emplace_back(p2, colour);
			m_ActiveLines.emplace_back(p3, colour);

			m_IndexBuffer.emplace_back(idx01);
			m_IndexBuffer.emplace_back(idx01 + 1);
			m_IndexBuffer.emplace_back(idx01 + 1);
			m_IndexBuffer.emplace_back(idx01 + 2);
			m_IndexBuffer.emplace_back(idx01 + 2);
			m_IndexBuffer.emplace_back(idx01 + 3);
			m_IndexBuffer.emplace_back(idx01 + 3);
			m_IndexBuffer.emplace_back(idx01);
		}
	}
}
