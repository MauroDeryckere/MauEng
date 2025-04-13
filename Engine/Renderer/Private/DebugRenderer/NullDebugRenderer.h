#ifndef MAUREN_NULLDEBUGRENDERER_H
#define MAUREN_NULLDEBUGRENDERER_H

#include "DebugRenderer.h"

namespace MauRen
{
	class NullDebugRenderer final : public DebugRenderer
	{
	public:
		NullDebugRenderer() = default;
		virtual ~NullDebugRenderer() override = default;

		virtual void DrawLine(glm::vec3 const& start, glm::vec3 const& end, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}

		virtual void DrawRect(glm::vec3 const& center, glm::vec2 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}
		virtual void DrawCube(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}

		virtual void DrawTriangle(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}

		virtual void DrawArrow(glm::vec3 const& start, glm::vec3 const& end, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, float arrowHeadLength = .5f, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}

		virtual void DrawPolygon(std::vector<glm::vec3> const& points, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}

		virtual void DrawCircle(glm::vec3 const& center, float radius, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}
		virtual void DrawEllipse(glm::vec3 const& center, glm::vec2 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}

		virtual void DrawSphere(glm::vec3 const& center, float radius, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}
		virtual void DrawSphereComplex(glm::vec3 const& center, float radius, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 4, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}
		virtual	void DrawEllipsoid(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}
		virtual	void DrawEllipsoidComplex(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 6, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}

		virtual void DrawCylinder(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept override {}
	};
}

#endif