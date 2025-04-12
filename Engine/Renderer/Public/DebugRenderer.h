#ifndef MAUREN_DEBUGRENDERER_H
#define MAUREN_DEBUGRENDERER_H

#include "Math/Rotator.h"

namespace MauRen
{
	class DebugRenderer
	{
	public:
		virtual ~DebugRenderer() = default;

		virtual void DrawLine(glm::vec3 const& start, glm::vec3 const& end, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;

		virtual void DrawRect(glm::vec3 const& center, glm::vec2 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;
		virtual void DrawCube(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;

		virtual void DrawTriangle(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;

		virtual void DrawArrow(glm::vec3 const& start, glm::vec3 const& end, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, float arrowHeadLength = .5f, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;

		virtual void DrawPolygon(std::vector<glm::vec3> const& points, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;

		virtual void DrawCircle(glm::vec3 const& center, float radius, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;
		virtual void DrawEllipse(glm::vec3 const& center, glm::vec2 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;

		virtual void DrawSphere(glm::vec3 const& center, float radius, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;
		virtual void DrawSphereComplex(glm::vec3 const& center, float radius, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 4, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;
		virtual	void DrawEllipsoid(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;
		virtual	void DrawEllipsoidComplex(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 6, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;

		virtual void DrawCylinder(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, bool isPivotOverride = false, glm::vec3 const& pivot = {}) noexcept = 0;

	protected:
		DebugRenderer() = default;
	};
}

#endif