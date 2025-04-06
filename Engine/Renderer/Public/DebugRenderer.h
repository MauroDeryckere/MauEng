#ifndef MAUREN_DEBUGRENDERER_H
#define MAUREN_DEBUGRENDERER_H

#include "DebugVertex.h"

namespace MauRen
{
	class Renderer;

	// Debug renderer class allows rendering debug shapes with a single call per tick
	// Most shapes currentl't don't support rotation, can be added in the future if necessary
	class DebugRenderer final
	{
	public:
		explicit DebugRenderer();
		~DebugRenderer() = default;

		void DrawLine(glm::vec3 const& start, glm::vec3 const& end, glm::vec3 const& colour = { 1, 0, 0 }) noexcept;

		void DrawRect(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& p3, glm::vec3 const& colour = { 1, 0, 0 }) noexcept;
		void DrawCube(glm::vec3 const& center, float size, glm::vec3 const& colour = { 1, 0, 0 }) noexcept;
		void DrawCube(glm::vec3 const& center, float width, float height, float depth, glm::vec3 const& colour = { 1, 0, 0 }) noexcept;

		void DrawTriangle(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& colour = { 1, 0, 0 }) noexcept;

		void DrawArrow(glm::vec3 const& start, glm::vec3 const& end, glm::vec3 const& colour = { 1, 0, 0 }, float arrowHeadLength = .5f) noexcept;

		void DrawPolygon(std::vector<glm::vec3> const& points, glm::vec3 const& colour = { 1, 0, 0 }) noexcept;

		void DrawCircle(glm::vec3 const& center, float radius, glm::vec3 const& axis, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept;
		void DrawEllipse(glm::vec3 const& center, float radiusX, float radiusY, glm::vec3 const& axis, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept;

		void DrawCylinder(glm::vec3 const& center, float radius, float height, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept;

		void DrawSphere(glm::vec3 const& center, float radius, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept;
		void DrawSphereComplex(glm::vec3 const& center, float radius, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 4) noexcept;
		void DrawEllipsoid(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept;
		void DrawEllipsoidComplex(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 6) noexcept;


		DebugRenderer(DebugRenderer const&) = delete;
		DebugRenderer(DebugRenderer&&) = delete;
		DebugRenderer& operator=(DebugRenderer const&) = delete;
		DebugRenderer& operator=(DebugRenderer&&) = delete;

	private:
		// Needs access to the variables during the render
		friend class VulkanRenderer;

		// Currently index buffer is not really being used optimally,
		// this can be improved but may not be worth spending a lot of time on since its used for debug only.
		std::vector<DebugVertex> m_ActivePoints{};
		std::vector<uint32_t> m_IndexBuffer;


		uint32_t const MAX_LINES{ 10'000 };
	};
}

#endif // MAUREN_DEBUGRENDERER_H