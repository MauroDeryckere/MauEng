#ifndef MAUREN_INTERNALDEBUGRENDERER_H
#define MAUREN_INTERNALDEBUGRENDERER_H

#include "DebugVertex.h"
#include "DebugRenderer.h"

namespace MauRen
{
	class Renderer;

	// Debug renderer class allows rendering debug shapes with a single call per tick
	// Most shapes currentl't don't support rotation, can be added in the future if necessary
	class InternalDebugRenderer final : public DebugRenderer
	{
	public:
		explicit InternalDebugRenderer();
		~InternalDebugRenderer() override = default;

		void DrawLine(glm::vec3 const& start, glm::vec3 const& end, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override;

		void DrawRect(glm::vec3 const& center, float width, float height, glm::vec3 const& axis = { 1, 0, 0 }, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override;
		void DrawCube(glm::vec3 const& center, float size, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override;
		void DrawCube(glm::vec3 const& center, float width, float height, float depth, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override;

		void DrawTriangle(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override;

		void DrawArrow(glm::vec3 const& start, glm::vec3 const& end, glm::vec3 const& colour = { 1, 0, 0 }, float arrowHeadLength = .5f) noexcept override;

		void DrawPolygon(std::vector<glm::vec3> const& points, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override;

		void DrawCircle(glm::vec3 const& center, float radius, glm::vec3 const& axis, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept override;
		void DrawEllipse(glm::vec3 const& center, float radiusX, float radiusY, glm::vec3 const& axis, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept override;

		void DrawCylinder(glm::vec3 const& center, float radius, float height, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept override;

		void DrawSphere(glm::vec3 const& center, float radius, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept override;
		void DrawSphereComplex(glm::vec3 const& center, float radius, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 4) noexcept override;
		void DrawEllipsoid(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept override;
		void DrawEllipsoidComplex(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 6) noexcept override;

		InternalDebugRenderer(InternalDebugRenderer const&) = delete;
		InternalDebugRenderer(InternalDebugRenderer&&) = delete;
		InternalDebugRenderer& operator=(InternalDebugRenderer const&) = delete;
		InternalDebugRenderer& operator=(InternalDebugRenderer&&) = delete;

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