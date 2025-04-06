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

		void DrawLine(glm::vec3 const& start, glm::vec3 const& end, glm::vec3 const& colour) noexcept;
		void DrawLine(DebugVertex const& start, DebugVertex const& end) noexcept;

		void DrawRect(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& p3, glm::vec3 const& colour = { 1, 0, 0 }) noexcept;

		DebugRenderer(DebugRenderer const&) = delete;
		DebugRenderer(DebugRenderer&&) = delete;
		DebugRenderer& operator=(DebugRenderer const&) = delete;
		DebugRenderer& operator=(DebugRenderer&&) = delete;

	private:
		// Needs access to the variables during the render
		friend class VulkanRenderer;

		std::vector<DebugVertex> m_ActiveLines{};
		std::vector<uint32_t> m_IndexBuffer;

		uint32_t const MAX_LINES{ 1'000 };
	};
}

#endif // MAUREN_DEBUGRENDERER_H