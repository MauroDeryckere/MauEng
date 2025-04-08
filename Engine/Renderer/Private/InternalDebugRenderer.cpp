#include "InternalDebugRenderer.h"

namespace MauRen
{
	InternalDebugRenderer::InternalDebugRenderer()
	{
		m_ActivePoints.reserve(MAX_LINES / 2);
		m_IndexBuffer.reserve(MAX_LINES / 2);
	}

	void InternalDebugRenderer::DrawLine(glm::vec3 const& start, glm::vec3 const& end, glm::vec3 const& colour) noexcept
	{
		if (std::size(m_ActivePoints) + 2 * 2 < MAX_LINES)
		{
			m_IndexBuffer.emplace_back(static_cast<uint32_t>(m_ActivePoints.size()));
			m_ActivePoints.emplace_back(start, colour);
			m_IndexBuffer.emplace_back(static_cast<uint32_t>(m_ActivePoints.size()));
			m_ActivePoints.emplace_back(end, colour);
		}
	}

	void InternalDebugRenderer::DrawRect(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& p3, glm::vec3 const& colour) noexcept
	{
		if (std::size(m_ActivePoints) + 4 * 2 < MAX_LINES)
		{
			DrawLine(p0, p1, colour);
			DrawLine(p1, p2, colour);
			DrawLine(p2, p3, colour);
			DrawLine(p3, p0, colour);
		}
	}

	void InternalDebugRenderer::DrawCube(glm::vec3 const& center, float size, glm::vec3 const& colour) noexcept
	{
		DrawCube(center, size, size, size, colour);
	}

	void InternalDebugRenderer::DrawCube(glm::vec3 const& center, float width, float height, float depth, glm::vec3 const& colour) noexcept
	{
		if (std::size(m_ActivePoints) + 12 * 2 < MAX_LINES)
		{
			
		}
		float const horHalfSize{ width * .5f };
		float const verHalfSize{ height * .5f };
		float const depthHalfSize{ depth * .5f };


		glm::vec3 const p0{ center + glm::vec3{-horHalfSize, -verHalfSize, -depthHalfSize} };
		glm::vec3 const p1{ center + glm::vec3{horHalfSize, -verHalfSize, -depthHalfSize} };
		glm::vec3 const p2{ center + glm::vec3{horHalfSize, -verHalfSize, depthHalfSize } };
		glm::vec3 const p3{ center + glm::vec3{-horHalfSize, -verHalfSize, depthHalfSize} };

		glm::vec3 const p4{ center + glm::vec3{-horHalfSize, verHalfSize, -depthHalfSize} };
		glm::vec3 const p5{ center + glm::vec3{horHalfSize, verHalfSize, -depthHalfSize} };
		glm::vec3 const p6{ center + glm::vec3{horHalfSize, verHalfSize, depthHalfSize} };
		glm::vec3 const p7{ center + glm::vec3{-horHalfSize, verHalfSize, depthHalfSize} };

		// Bottom face
		DrawLine(p0, p1, colour);
		DrawLine(p1, p2, colour);
		DrawLine(p2, p3, colour);
		DrawLine(p3, p0, colour);

		// Top face
		DrawLine(p4, p5, colour);
		DrawLine(p5, p6, colour);
		DrawLine(p6, p7, colour);
		DrawLine(p7, p4, colour);

		// Vertical edges
		DrawLine(p0, p4, colour);
		DrawLine(p1, p5, colour);
		DrawLine(p2, p6, colour);
		DrawLine(p3, p7, colour);
	}

	void InternalDebugRenderer::DrawTriangle(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& colour) noexcept
	{
		if (std::size(m_ActivePoints) + 3 * 2 < MAX_LINES)
		{
			DrawLine(p0, p1, colour);
			DrawLine(p1, p2, colour);
			DrawLine(p2, p1, colour);
		}
	}

	void InternalDebugRenderer::DrawArrow(glm::vec3 const& start, glm::vec3 const& end, glm::vec3 const& colour, float arrowHeadLength) noexcept
	{
		if (std::size(m_ActivePoints) + 3 * 2 < MAX_LINES)
		{
			DrawLine(start, end, colour);

			glm::vec3 const direction{ glm::normalize(end - start) };
			float constexpr arrowheadAngle{ glm::pi<float>() / 6.0f };

			glm::vec3 const right{ glm::normalize(glm::cross(direction, glm::vec3{ 0.0f, 1.0f, 0.0f })) * arrowHeadLength };

			glm::quat const rotationRight(glm::angleAxis(arrowheadAngle, right));
			glm::quat const rotationLeft(glm::angleAxis(-arrowheadAngle, right));

			// Apply the rotations to the direction vector
			glm::vec3 const arrowhead1{ end - rotationRight * direction * arrowHeadLength };
			glm::vec3 const arrowhead2{ end - rotationLeft * direction * arrowHeadLength };

			// Draw the two lines forming the arrowhead
			DrawLine(end, arrowhead1, colour);
			DrawLine(end, arrowhead2, colour);
		}
	}

	void InternalDebugRenderer::DrawPolygon(std::vector<glm::vec3> const& points, glm::vec3 const& colour) noexcept
	{
		assert(points.size() > 3);
		if (points.size() < 3)
		{
			return;
		}

		for (size_t i{ 0 }; i < points.size(); ++i)
		{
			glm::vec3 const& p1 = points[i];
			glm::vec3 const& p2 = points[(i + 1) % points.size()];

			DrawLine(p1, p2, colour);
		}
	}

	void InternalDebugRenderer::DrawCircle(glm::vec3 const& center, float radius, glm::vec3 const& axis, glm::vec3 const& colour, uint32_t segments) noexcept
	{
		if (std::size(m_ActivePoints) + (segments) * 2 < MAX_LINES)
		{
			float const delta{ glm::two_pi<float>() / static_cast<float>(segments) };

			// Pick an arbitrary perpendicular vector for rotation
			glm::vec3 v1{ glm::normalize(glm::abs(axis.x) > 0.99f ? glm::vec3{0, 1, 0} : glm::vec3{1, 0, 0}) };
			glm::vec3 const v2{ glm::normalize(glm::cross(axis, v1)) };
			v1 = glm::normalize(glm::cross(v2, axis));

			for (uint32_t i{ 0 }; i < segments; ++i)
			{
				float const angle0{ i * delta };
				float const angle1{ (i + 1) * delta };

				glm::vec3 const p0{ center + radius * (glm::cos(angle0) * v1 + glm::sin(angle0) * v2) };
				glm::vec3 const p1{ center + radius * (glm::cos(angle1) * v1 + glm::sin(angle1) * v2) };

				DrawLine(p0, p1, colour);
			}
		}

	}

	void InternalDebugRenderer::DrawEllipse(glm::vec3 const& center, float radiusX, float radiusY, glm::vec3 const& axis, glm::vec3 const& colour, uint32_t segments) noexcept
	{
		if (std::size(m_ActivePoints) + (segments) * 2 < MAX_LINES)
		{
			float const delta{ glm::two_pi<float>() / static_cast<float>(segments) };

			// Pick an arbitrary perpendicular vector for rotation
			glm::vec3 v1{ glm::normalize(glm::abs(axis.x) > 0.99f ? glm::vec3{0, 1, 0} : glm::vec3{1, 0, 0}) };
			glm::vec3 const v2{ glm::normalize(glm::cross(axis, v1)) };
			v1 = glm::normalize(glm::cross(v2, axis));  // Final perpendicular vector to axis

			for (uint32_t i{ 0 }; i < segments; ++i)
			{
				float const angle0{ i * delta };
				float const angle1{ (i + 1) * delta };

				glm::vec3 const p0{ center + radiusX * glm::cos(angle0) * v1 + radiusY * glm::sin(angle0) * v2 };
				glm::vec3 const p1{ center + radiusX * glm::cos(angle1) * v1 + radiusY * glm::sin(angle1) * v2 };

				DrawLine(p0, p1, colour);
			}
		}
	}

	void InternalDebugRenderer::DrawCylinder(glm::vec3 const& center, float radius, float height, glm::vec3 const& colour, uint32_t segments) noexcept
	{
		if (std::size(m_ActivePoints) + 2 * ( 2 * segments + segments * 2) < MAX_LINES)
		{
			glm::vec3 const topCenter{ center + glm::vec3{ 0.0f, height * 0.5f, 0.0f } };
			glm::vec3 const bottomCenter{ center - glm::vec3{ 0.0f, height * 0.5f, 0.0f } };

			DrawCircle(topCenter, radius, { 0, 1, 0 }, colour, segments);
			DrawCircle(bottomCenter, radius, { 0, 1, 0 }, colour, segments);

			for (uint32_t i{ 0 }; i < segments; ++i)
			{
				float const angle1{ (2.0f * glm::pi<float>() * static_cast<float>(i)) / static_cast<float>(segments) };
				float const angle2{ (2.0f * glm::pi<float>() * static_cast<float>(i + 1)) / static_cast<float>(segments) };

				glm::vec3 const topPoint1{ topCenter + glm::vec3{ radius * glm::cos(angle1), 0.0f, radius * glm::sin(angle1) } };
				glm::vec3 const topPoint2{ topCenter + glm::vec3{ radius * glm::cos(angle2), 0.0f, radius * glm::sin(angle2) } };

				glm::vec3 const bottomPoint1{ bottomCenter + glm::vec3{ radius * glm::cos(angle1), 0.0f, radius * glm::sin(angle1) } };
				glm::vec3 const bottomPoint2{ bottomCenter + glm::vec3{ radius * glm::cos(angle2), 0.0f, radius * glm::sin(angle2) } };

				DrawLine(topPoint1, bottomPoint1, colour);
				DrawLine(topPoint2, bottomPoint2, colour);
			}
		}
	}

	void InternalDebugRenderer::DrawSphere(glm::vec3 const& center, float radius, glm::vec3 const& colour, uint32_t segments) noexcept
	{
		if (std::size(m_ActivePoints) + (segments * 3) * 2 < MAX_LINES)
		{
			DrawCircle(center, radius, glm::vec3{ 1, 0, 0 }, colour, segments); // YZ plane
			DrawCircle(center, radius, glm::vec3{ 0, 1, 0 }, colour, segments); // XZ plane
			DrawCircle(center, radius, glm::vec3{ 0, 0, 1 }, colour, segments); // XY plane
		}
	}

	void InternalDebugRenderer::DrawSphereComplex(glm::vec3 const& center, float radius, glm::vec3 const& colour, uint32_t segments, uint32_t layers) noexcept
	{
		if (std::size(m_ActivePoints) + ((segments * 3) * (layers * 2 - 1)) * 2 < MAX_LINES)
		{
			// Horizontal rings from pole to pole (latitude)
			for (uint32_t i{ 1 }; i < layers; ++i)
			{
				// 0 - PI
				float const theta{ glm::pi<float>() * static_cast<float>(i) / static_cast<float>(layers) };

				float const height{ glm::cos(theta) };
				// radius of current ring
				float const rad{ glm::sin(theta) };

				glm::vec3 const ringCenter{ center + glm::vec3{0.0f, height * radius, 0.0f} };

				// XY rings along Y
				DrawCircle(ringCenter, rad * radius, glm::vec3{ 0, 1, 0 }, colour, segments);
			}

			// Vertical rings (longitude)
			for (uint32_t i{ 0 }; i < layers; ++i)
			{
				float const phi{ glm::pi<float>() * static_cast<float>(i) / static_cast<float>(layers) };
				glm::vec3 const axis{ glm::vec3{glm::cos(phi), 0.0f, glm::sin(phi)} };

				DrawCircle(center, radius, axis, colour, segments);
			}
		}
	}

	void InternalDebugRenderer::DrawEllipsoid(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour, uint32_t segments) noexcept
	{
		if (std::size(m_ActivePoints) + (segments * 3) * 2 < MAX_LINES)
		{
			DrawEllipse(center, radiusX, radiusY, glm::vec3{ 0, 0, 1 }, colour, segments); // XZ plane
			DrawEllipse(center, radiusX, radiusZ, glm::vec3{ 0, 1, 0 }, colour, segments); // XZ plane
			DrawEllipse(center, radiusY, radiusZ, glm::vec3{ 1, 0, 0 }, colour, segments); // YZ plane
		}
	}

	void InternalDebugRenderer::DrawEllipsoidComplex(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour, uint32_t segments, uint32_t layers) noexcept
	{
		if (std::size(m_ActivePoints) + ((segments * 3) * (layers * 2 - 1)) * 2 < MAX_LINES)
		{
			// Horizontal rings from pole to pole (latitude)
			for (uint32_t i{ 1 }; i < layers; ++i)
			{
				// 0 - PI
				float const theta{ glm::pi<float>() * static_cast<float>(i) / static_cast<float>(layers) };
				float const height{ glm::cos(theta) * radiusY };

				glm::vec3 const ringCenter{ center + glm::vec3{ 0.0f, height, 0.0f } };

				DrawEllipse(ringCenter, glm::sin(theta) * radiusX, glm::sin(theta) * radiusZ, glm::vec3{ 0, 1, 0 }, colour, segments);
			}

			// Vertical rings (longitude)
			for (uint32_t i{ 0 }; i < layers; ++i)
			{
				float const phi{ glm::pi<float>() * static_cast<float>(i) / static_cast<float>(layers) };
				glm::vec3 const axis{ glm::vec3{glm::cos(phi), glm::sin(phi), 0} };

				DrawEllipse(center, radiusY * glm::cos(phi), radiusZ, axis, colour, segments);
			}
		}
	}
}
