#ifndef MAUREN_DEBUGRENDERER_H
#define MAUREN_DEBUGRENDERER_H

#include "Math/Rotator.h"

namespace MauRen
{
	class DebugRenderer
	{
	public:
		virtual ~DebugRenderer() = default;

		virtual void DrawLine(glm::vec3 const& start, glm::vec3 const& end, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }) noexcept = 0;

		virtual void DrawRect(glm::vec3 const& center, float width, float height, glm::vec3 const& axis = { 1, 0, 0 }, glm::vec3 const& colour = { 1, 0, 0 }) noexcept = 0;
		virtual void DrawCube(glm::vec3 const& center, float size, glm::vec3 const& colour = { 1, 0, 0 }) noexcept = 0;
		virtual void DrawCube(glm::vec3 const& center, float width, float height, float depth, glm::vec3 const& colour = { 1, 0, 0 }) noexcept = 0;

		virtual void DrawTriangle(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& colour = { 1, 0, 0 }) noexcept = 0;

		virtual void DrawArrow(glm::vec3 const& start, glm::vec3 const& end, glm::vec3 const& colour = { 1, 0, 0 }, float arrowHeadLength = .5f) noexcept = 0;

		virtual void DrawPolygon(std::vector<glm::vec3> const& points, glm::vec3 const& colour = { 1, 0, 0 }) noexcept = 0;

		virtual void DrawCircle(glm::vec3 const& center, float radius, glm::vec3 const& axis, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept = 0;
		virtual void DrawEllipse(glm::vec3 const& center, float radiusX, float radiusY, glm::vec3 const& axis, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept = 0;

		virtual void DrawCylinder(glm::vec3 const& center, float radius, float height, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept = 0;

		virtual void DrawSphere(glm::vec3 const& center, float radius, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept = 0;
		virtual void DrawSphereComplex(glm::vec3 const& center, float radius, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 4) noexcept = 0;
		virtual	void DrawEllipsoid(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept = 0;
		virtual	void DrawEllipsoidComplex(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 6) noexcept = 0;

	protected:
		explicit DebugRenderer() = default;

	private:
	};

	class NullDebugRenderer final : public DebugRenderer
	{
	public:
		explicit NullDebugRenderer() = default;
		virtual ~NullDebugRenderer() override = default;

		virtual void DrawLine(glm::vec3 const& start, glm::vec3 const& end, MauCor::Rotator const& rot = {}, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override {}

		virtual void DrawRect(glm::vec3 const& center, float width, float height, glm::vec3 const& axis = { 1, 0, 0 }, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override {}
		virtual void DrawCube(glm::vec3 const& center, float size, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override {}
		virtual void DrawCube(glm::vec3 const& center, float width, float height, float depth, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override {}

		virtual void DrawTriangle(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override {}

		virtual void DrawArrow(glm::vec3 const& start, glm::vec3 const& end, glm::vec3 const& colour = { 1, 0, 0 }, float arrowHeadLength = .5f) noexcept override {}

		virtual void DrawPolygon(std::vector<glm::vec3> const& points, glm::vec3 const& colour = { 1, 0, 0 }) noexcept override {}

		virtual void DrawCircle(glm::vec3 const& center, float radius, glm::vec3 const& axis, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept override {}
		virtual void DrawEllipse(glm::vec3 const& center, float radiusX, float radiusY, glm::vec3 const& axis, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept override {}

		virtual void DrawCylinder(glm::vec3 const& center, float radius, float height, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept override {}

		virtual void DrawSphere(glm::vec3 const& center, float radius, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept override {}
		virtual void DrawSphereComplex(glm::vec3 const& center, float radius, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 4) noexcept override {}
		virtual	void DrawEllipsoid(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24) noexcept override {}
		virtual	void DrawEllipsoidComplex(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour = { 1, 0, 0 }, uint32_t segments = 24, uint32_t layers = 6) noexcept override {}
	private:
	};
}

#endif