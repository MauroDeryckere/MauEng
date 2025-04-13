#ifndef MAUREN_INTERNALDEBUGRENDERER_H
#define MAUREN_INTERNALDEBUGRENDERER_H

#include "DebugRenderer.h"
#include "DebugVertex.h"

namespace MauRen
{
	class Renderer;

	// Debug renderer class allows rendering debug shapes with a single call per tick
	class InternalDebugRenderer final : public DebugRenderer
	{
	public:
		explicit InternalDebugRenderer();
		~InternalDebugRenderer() override = default;

		void DrawLine(glm::vec3 const& start, glm::vec3 const& end, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;

		void DrawRect(glm::vec3 const& center, glm::vec2 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;
		void DrawCube(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;

		void DrawTriangle(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;


		void DrawArrow(glm::vec3 const& start, glm::vec3 const& end, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, float arrowHeadLength = .5f, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;

		void DrawPolygon(std::vector<glm::vec3> const& points, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;

		void DrawCircle(glm::vec3 const& center, float radius, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;
		void DrawEllipse(glm::vec3 const& center, glm::vec2 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;

		void DrawSphere(glm::vec3 const& center, float radius, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;
		void DrawSphereComplex(glm::vec3 const& center, float radius, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 4, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;
		void DrawEllipsoid(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;
		void DrawEllipsoidComplex(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 6, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;

		void DrawCylinder(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override;


		InternalDebugRenderer(InternalDebugRenderer const&) = delete;
		InternalDebugRenderer(InternalDebugRenderer&&) = delete;
		InternalDebugRenderer& operator=(InternalDebugRenderer const&) = delete;
		InternalDebugRenderer& operator=(InternalDebugRenderer&&) = delete;

	private:
		// Needs access to the variables during the render
		friend class VulkanRenderer;

		// Currently index buffer is not really being used optimally,
		// this can be improved but may not be worth spending a lot of time on since its used for debug only.
		std::vector<DebugVertex> m_ActivePoints;
		std::vector<uint32_t> m_IndexBuffer;


		uint32_t const MAX_LINES{ 100'000 };

		template<typename TransformFunc>
		void AddDebugLines(
			std::vector<glm::vec3> const& localPoints,
			std::vector<std::pair<uint32_t, uint32_t>> const& lineIndices,
			TransformFunc&& transform,
			glm::vec3 const& color);
	};
}

#endif // MAUREN_DEBUGRENDERER_H