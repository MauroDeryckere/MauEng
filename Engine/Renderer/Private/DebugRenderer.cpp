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

	void DebugRenderer::DrawRect(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& p3, glm::vec3 const& colour) noexcept
	{
		if (std::size(m_ActiveLines) + 4 < MAX_LINES)
		{
			DrawLine(p0, p1, colour);
			DrawLine(p1, p2, colour);
			DrawLine(p2, p3, colour);
			DrawLine(p3, p0, colour);
		}
	}

	void DebugRenderer::DrawCube(glm::vec3 const& center, float size, glm::vec3 const& colour) noexcept
	{
		DrawCube(center, size, size, size, colour);
	}

	void DebugRenderer::DrawCube(glm::vec3 const& center, float width, float height, float depth, glm::vec3 const& colour) noexcept
	{
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

	void DebugRenderer::DrawCircle(glm::vec3 const& center, float radius, glm::vec3 const& axis, glm::vec3 const& colour, uint32_t segments) noexcept
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

	void DebugRenderer::DrawEllipse(glm::vec3 const& center, float radiusX, float radiusY, glm::vec3 const& axis, glm::vec3 const& colour, uint32_t segments) noexcept
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

	void DebugRenderer::DrawSphere(glm::vec3 const& center, float radius, glm::vec3 const& colour, uint32_t segments) noexcept
	{
		if (std::size(m_ActiveLines) + (segments * 3) < MAX_LINES)
		{
			DrawCircle(center, radius, glm::vec3{ 1, 0, 0 }, colour, segments); // YZ plane
			DrawCircle(center, radius, glm::vec3{ 0, 1, 0 }, colour, segments); // XZ plane
			DrawCircle(center, radius, glm::vec3{ 0, 0, 1 }, colour, segments); // XY plane
		}
	}

	void DebugRenderer::DrawSphereComplex(glm::vec3 const& center, float radius, glm::vec3 const& colour, uint32_t segments, uint32_t layers) noexcept
	{
		if (std::size(m_ActiveLines) + ((segments * 3) *(layers * 2 - 1)) < MAX_LINES)
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

	void DebugRenderer::DrawEllipsoid(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour, uint32_t segments) noexcept
	{
		if (std::size(m_ActiveLines) + (segments * 3) < MAX_LINES)
		{
			DrawEllipse(center, radiusX, radiusY, glm::vec3{ 0, 0, 1 }, colour, segments); // XZ plane
			DrawEllipse(center, radiusX, radiusZ, glm::vec3{ 0, 1, 0 }, colour, segments); // XZ plane
			DrawEllipse(center, radiusY, radiusZ, glm::vec3{ 1, 0, 0 }, colour, segments); // YZ plane
		}
	}

	void DebugRenderer::DrawEllipsoidComplex(glm::vec3 const& center, float radiusX, float radiusY, float radiusZ, glm::vec3 const& colour, uint32_t segments, uint32_t layers) noexcept
	{
		if (std::size(m_ActiveLines) + ((segments * 3) * (layers * 2 - 1)) < MAX_LINES)
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
