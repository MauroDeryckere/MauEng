#include "InternalDebugRenderer.h"

#include "Math/Rotator.h"

namespace MauRen
{
	InternalDebugRenderer::InternalDebugRenderer()
	{
		m_ActivePoints.reserve(MAX_LINES / 2);
		m_IndexBuffer.reserve(MAX_LINES / 2);
	}

	void InternalDebugRenderer::DrawLine(glm::vec3 const& start, glm::vec3 const& end, MauCor::Rotator const& rot, glm::vec3 const& colour) noexcept
	{
		if (std::size(m_ActivePoints) + 2 < MAX_LINES)
		{
			m_IndexBuffer.emplace_back(static_cast<uint32_t>(m_ActivePoints.size()));
			m_ActivePoints.emplace_back(glm::rotate(rot.rotation, start), colour);
			m_IndexBuffer.emplace_back(static_cast<uint32_t>(m_ActivePoints.size()));
			m_ActivePoints.emplace_back(glm::rotate(rot.rotation, end), colour);
		}
		else
		{
			ME_LOG_WARN(MauCor::LogCategory::Renderer, "Debug renderer active points has surpassed the set limit, edit the config or try drawing less points! ");
		}
	}

	void InternalDebugRenderer::DrawRect(glm::vec3 const& center, glm::vec2 const& size, MauCor::Rotator const& rot, glm::vec3 const& colour) noexcept
	{
		// Points: 
		// 01 --- 02
		// |	  |
		// |	  |
		// 03 --- 04

		// Indices:
		// 01 -> 02
		// 02 -> 04
		// 04 -> 03
		// 03 -> 01

		glm::vec2 const halfSize{ size * 0.5f };

		std::vector<glm::vec3> const localPoints
		{
			{ -halfSize.x, -halfSize.y, 0},
			{ halfSize.x, -halfSize.y, 0},
			{ halfSize.x, halfSize.y, 0},
			{ -halfSize.x, halfSize.y, 0}
		};

		std::vector<std::pair<uint32_t, uint32_t>> const lines
		{
			{0, 1}, {1, 2}, {2, 3}, {3, 0}
		};

		AddDebugLines(localPoints, lines, rot.rotation, colour, center);
	}

	void InternalDebugRenderer::DrawCube(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot, glm::vec3 const& colour) noexcept
	{
		float const horHalfSize{ size.x *.5f };
		float const verHalfSize{ size.y * .5f };
		float const depthHalfSize{ size.z * .5f };

		std::vector<glm::vec3> const localPoints
		{
			glm::vec3 { glm::vec3{-horHalfSize, -verHalfSize, -depthHalfSize} },
			glm::vec3 { glm::vec3{horHalfSize, -verHalfSize, -depthHalfSize} },
			glm::vec3 { glm::vec3{horHalfSize, -verHalfSize, depthHalfSize } },
			glm::vec3 { glm::vec3{-horHalfSize, -verHalfSize, depthHalfSize} },

			glm::vec3 { glm::vec3{-horHalfSize, verHalfSize, -depthHalfSize} },
			glm::vec3 { glm::vec3{horHalfSize, verHalfSize, -depthHalfSize} },
			glm::vec3 { glm::vec3{horHalfSize, verHalfSize, depthHalfSize} },
			glm::vec3 { glm::vec3{-horHalfSize, verHalfSize, depthHalfSize} }
		};


		std::vector<std::pair<uint32_t, uint32_t>> const lines 
		{
			// Bottom face edges
			{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },

			// Top face edges
			{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },

			// Connecting lines between top and bottom faces
			{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
		};

		AddDebugLines(localPoints, lines, rot.rotation, colour, center);
	}

	void InternalDebugRenderer::DrawTriangle(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, MauCor::Rotator const& rot, glm::vec3 const& colour) noexcept
	{
		glm::vec3 const center{ (p0 + p1 + p2) / 3.0f };
		std::vector<glm::vec3> const localPoints
		{
			glm::vec3 { p0 - center },
			glm::vec3 { p1 - center },
			glm::vec3 { p2 - center }
		};

		std::vector<std::pair<uint32_t, uint32_t>> const lines
		{
			{0, 1}, {1, 2}, {2, 0}
		};

		AddDebugLines(localPoints, lines, rot.rotation, colour, center);
	}

	void InternalDebugRenderer::DrawArrow(glm::vec3 const& start, glm::vec3 const& end, MauCor::Rotator const& rot, glm::vec3 const& colour, float arrowHeadLength) noexcept
	{
		glm::vec3 const direction{ glm::normalize(end - start) };
		float constexpr arrowheadAngle{ glm::pi<float>() / 6.0f };

		glm::vec3 const right{ glm::normalize(glm::cross(direction, glm::vec3{ 0.0f, 1.0f, 0.0f })) * arrowHeadLength };

		glm::quat const rotationRight(glm::angleAxis(arrowheadAngle, right));
		glm::quat const rotationLeft(glm::angleAxis(-arrowheadAngle, right));

		// Apply the rotations to the direction vector
		glm::vec3 const arrowhead1{ end - rotationRight * direction * arrowHeadLength };
		glm::vec3 const arrowhead2{ end - rotationLeft * direction * arrowHeadLength };

		auto const center{ (end - start) *.5f};

		std::vector<glm::vec3> const localPoints
		{
			glm::vec3 { start - center },
			glm::vec3 { end - center },
			glm::vec3 { arrowhead1 - center },
			glm::vec3 { arrowhead2 - center }
		};

		std::vector<std::pair<uint32_t, uint32_t>> const lines
		{
			{0, 1}, {1, 2}, {1, 3}
		};

		AddDebugLines(localPoints, lines, rot.rotation, colour, center);
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
			//glm::vec3 const& p1 = points[i];
			//glm::vec3 const& p2 = points[(i + 1) % points.size()];

			//DrawLine(p1, p2, colour);
		}
	}

	void InternalDebugRenderer::DrawCircle(glm::vec3 const& center, float radius, MauCor::Rotator const& rot, glm::vec3 const& colour, uint32_t segments) noexcept
	{
		DrawEllipse(center, { radius, radius }, rot, colour, segments);
	}

	void InternalDebugRenderer::DrawEllipse(glm::vec3 const& center, glm::vec2 const& size, MauCor::Rotator const& rot, glm::vec3 const& colour, uint32_t segments) noexcept
	{
		std::vector<glm::vec3> points{};
		std::vector<std::pair<uint32_t, uint32_t>> indices{};
		DrawEllipseLocal(size, points, indices, segments);

		AddDebugLines(points, indices, rot.rotation, colour, center);
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

				//DrawLine(topPoint1, bottomPoint1, colour);
				//DrawLine(topPoint2, bottomPoint2, colour);
			}
		}
	}

	void InternalDebugRenderer::DrawSphere(glm::vec3 const& center, float radius, MauCor::Rotator const& rot, glm::vec3 const& colour, uint32_t segments) noexcept
	{
		{
			std::vector<glm::vec3> points{};
			std::vector<std::pair<uint32_t, uint32_t>> indices{};
			DrawEllipseLocal({ radius , radius }, points, indices, segments);

			for (auto& p : points)
			{
				p = (MauCor::Rotator{ 0, 90, 0 } * p);
			}

			AddDebugLines(points, indices, rot.rotation, colour, center);
		}

		
		{
			std::vector<glm::vec3> points{};
			std::vector<std::pair<uint32_t, uint32_t>> indices{};
			DrawEllipseLocal({ radius , radius }, points, indices, segments);

			for (auto& p : points)
			{
				p = (MauCor::Rotator{ 0, 0, 90 }  * p);
			}

			AddDebugLines(points, indices, rot.rotation, colour, center);
		}
			
		{
			std::vector<glm::vec3> points{};
			std::vector<std::pair<uint32_t, uint32_t>> indices{};
			DrawEllipseLocal({ radius , radius }, points, indices, segments);

			for (auto& p : points)
			{
				p = (MauCor::Rotator{ 90, 0, 0 }  * p);
			}

			AddDebugLines(points, indices, rot.rotation, colour, center);
		}
	}

	void InternalDebugRenderer::DrawSphereComplex(glm::vec3 const& center, float radius, MauCor::Rotator const& rot, glm::vec3 const& colour, uint32_t segments, uint32_t layers) noexcept
	{
		ME_PROFILE_FUNCTION()

		// Horizontal rings from pole to pole (latitude)
		for (uint32_t i{ 1 }; i < layers; ++i)
		{
			// 0 - PI
			float const theta{ glm::pi<float>() * static_cast<float>(i) / static_cast<float>(layers) };

			float const height{ glm::cos(theta) };
			// radius of current ring
			float const rad{ glm::sin(theta) };

			glm::vec3 const ringCenter{ center + glm::vec3{0.0f, height * radius, 0.0f} };
			MauCor::Rotator const localRot{ 90, 0, 0 };

			// XY rings along Y

			std::vector<glm::vec3> points{};
			std::vector<std::pair<uint32_t, uint32_t>> indices{};
			DrawEllipseLocal(glm::vec2 { rad* radius, rad* radius }, points, indices, segments);

			for (auto & p : points)
			{
				p = (ringCenter + localRot.rotation * p) - center;
			}

			AddDebugLines(points, indices, rot.rotation , colour, center);
		}

		// Vertical rings (longitude)
		for (uint32_t i{ 0 }; i < layers; ++i)
		{
			float const phi{ 180.f * static_cast<float>(i) / static_cast<float>(layers) };
			MauCor::Rotator const localRot{ 0.0f, phi, 0.0f };

			std::vector<glm::vec3> points{};
			std::vector<std::pair<uint32_t, uint32_t>> indices{};
			DrawEllipseLocal(glm::vec2{  radius,  radius }, points, indices, segments);

			for (auto& p : points)
			{
				p = localRot.rotation * p;
			}

			AddDebugLines(points, indices, rot.rotation, colour, center);
		}
	}

	void InternalDebugRenderer::DrawSphereComplex_Parametric(glm::vec3 const& center, float radius,
		MauCor::Rotator const& rot, glm::vec3 const& colour, uint32_t segments, uint32_t layers) noexcept
	{
		ME_PROFILE_FUNCTION()

		std::vector<std::pair<uint32_t, uint32_t>> indices;
		auto const baseId{ m_ActivePoints.size() };
		uint32_t const indexStride{ segments + 1 };

		// Generate grid of points
		for (uint32_t i = 0; i <= layers; ++i)
		{
			float theta = glm::pi<float>() * static_cast<float>(i) / static_cast<float>(layers); // 0 to PI (latitude)
			float sinTheta = std::sin(theta);
			float cosTheta = std::cos(theta);

			for (uint32_t j = 0; j <= segments; ++j)
			{
				float phi = glm::two_pi<float>() * static_cast<float>(j) / static_cast<float>(segments); // 0 to 2PI (longitude)
				float sinPhi = std::sin(phi);
				float cosPhi = std::cos(phi);

				glm::vec3 localPoint{
					radius * sinTheta * cosPhi,
					radius * cosTheta,
					radius * sinTheta * sinPhi
				};


				m_ActivePoints.emplace_back(rot.rotation * localPoint + center, colour);

				if (j > 0)
				{
					m_IndexBuffer.emplace_back(baseId + (i * indexStride + j - 1));
					m_IndexBuffer.emplace_back(baseId + (i * indexStride + j));
				}

				if (i > 0)
				{
					m_IndexBuffer.emplace_back(baseId + ((i - 1) * indexStride + j));
					m_IndexBuffer.emplace_back(baseId + (i * indexStride + j));
				}
			}
		}
	}

	void InternalDebugRenderer::DrawEllipsoid(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot, glm::vec3 const& colour, uint32_t segments) noexcept
	{
		{
			std::vector<glm::vec3> points{};
			std::vector<std::pair<uint32_t, uint32_t>> indices{};
			DrawEllipseLocal({ size.x , size.z }, points, indices, segments);

			for (auto& p : points)
			{
				p = MauCor::Rotator{ 0, 90, 0 } * p;
			}

			AddDebugLines(points, indices, rot.rotation, colour, center);
		}

		{
			std::vector<glm::vec3> points{};
			std::vector<std::pair<uint32_t, uint32_t>> indices{};
			DrawEllipseLocal({ size.x , size.y }, points, indices, segments);

			for (auto& p : points)
			{
				p = MauCor::Rotator{ 0, 0, 90 } * p;
			}

			AddDebugLines(points, indices, rot.rotation, colour, center);
		}

		{
			std::vector<glm::vec3> points{};
			std::vector<std::pair<uint32_t, uint32_t>> indices{};
			DrawEllipseLocal({ size.y , size.z }, points, indices, segments);

			for (auto& p : points)
			{
				p = MauCor::Rotator{ 90, 0, 0 } * p;
			}

			AddDebugLines(points, indices, rot.rotation, colour, center);
		}
	}

	void InternalDebugRenderer::DrawEllipsoidComplex(glm::vec3 const& center, glm::vec3 const& size, MauCor::Rotator const& rot, glm::vec3 const& colour, uint32_t segments, uint32_t layers) noexcept
	{
		ME_PROFILE_FUNCTION()

		// Horizontal rings from pole to pole (latitude)
		for (uint32_t i{ 1 }; i < layers; ++i)
		{
			// 0 - PI
			float const theta{ glm::pi<float>() * static_cast<float>(i) / static_cast<float>(layers) };
			float const height{ glm::cos(theta) * size.y };

			glm::vec3 const ringCenter{ center + glm::vec3{ 0.0f, height, 0.0f } };

			std::vector<glm::vec3> points{};
			std::vector<std::pair<uint32_t, uint32_t>> indices{};
			DrawEllipseLocal(glm::vec2{ glm::sin(theta) * size.x, glm::sin(theta) * size.z }, points, indices, segments);

			MauCor::Rotator const localRot{ 90, 0 , 0 };

			for (auto& p : points)
			{
				p = (ringCenter + localRot.rotation * p) - center;
			}

			AddDebugLines(points, indices, rot.rotation, colour, center);
		}

		// Vertical rings (longitude)
		for (uint32_t i{ 0 }; i < layers; ++i)
		{
			float const phi{ 180.f * static_cast<float>(i) / static_cast<float>(layers) };
			MauCor::Rotator const localRot{ 0 ,90 , phi };

			std::vector<glm::vec3> points{};
			std::vector<std::pair<uint32_t, uint32_t>> indices{};
			DrawEllipseLocal(glm::vec2{ size.y, size.z }, points, indices, segments);

			for (auto& p : points)
			{
				p = (center + localRot.rotation * p) - center;
			}

			AddDebugLines(points, indices, MauCor::Rotator{90, 0, 0}.rotation * rot.rotation, colour, center);
		}
	}

	void InternalDebugRenderer::DrawEllipsoidComplex_Parametric(glm::vec3 const& center, glm::vec3 const& size,
		MauCor::Rotator const& rot, glm::vec3 const& colour, uint32_t segments, uint32_t layers) noexcept
	{
		ME_PROFILE_FUNCTION()

		std::vector<std::pair<uint32_t, uint32_t>> indices;
		auto const baseId{ m_ActivePoints.size() };
		uint32_t const indexStride{ segments + 1 };

		// Generate grid of points
		for (uint32_t i = 0; i <= layers; ++i)
		{
			float theta = glm::pi<float>() * static_cast<float>(i) / static_cast<float>(layers); // 0 to PI (latitude)
			float sinTheta = std::sin(theta);
			float cosTheta = std::cos(theta);

			for (uint32_t j = 0; j <= segments; ++j)
			{
				float phi = glm::two_pi<float>() * static_cast<float>(j) / static_cast<float>(segments); // 0 to 2PI (longitude)
				float sinPhi = std::sin(phi);
				float cosPhi = std::cos(phi);

				glm::vec3 localPoint{
					size.x * sinTheta * cosPhi,
					size.y * cosTheta,
					size.z * sinTheta * sinPhi
				};

				m_ActivePoints.emplace_back(rot.rotation * localPoint + center, colour);

				if (j > 0)
				{
					m_IndexBuffer.emplace_back(baseId + (i * indexStride + j - 1));
					m_IndexBuffer.emplace_back(baseId + (i * indexStride + j));
				}

				if (i > 0)
				{
					m_IndexBuffer.emplace_back(baseId + ((i - 1) * indexStride + j));
					m_IndexBuffer.emplace_back(baseId + (i * indexStride + j));
				}
			}
		}
	}

	void InternalDebugRenderer::AddDebugLines(std::vector<glm::vec3> const& localPoints,
	                                          std::vector<std::pair<uint32_t, uint32_t>> const& lineIndices, glm::quat const& rotation,
	                                          glm::vec3 const& color, glm::vec3 const& center)
	{
		ME_PROFILE_FUNCTION()
		if (std::size(m_ActivePoints) + localPoints.size() >= MAX_LINES)
		{
			ME_LOG_WARN(MauCor::LogCategory::Renderer, "Debug renderer active points has surpassed the set limit.");
			return;
		}

		auto const baseIndex{ static_cast<uint32_t>(m_ActivePoints.size()) };

		for (auto const& localPoint : localPoints)
		{
			m_ActivePoints.emplace_back(center + rotation * localPoint, color);
		}

		for (auto const& [start, end] : lineIndices)
		{
			m_IndexBuffer.emplace_back(baseIndex + start);
			m_IndexBuffer.emplace_back(baseIndex + end);
		}
	}

	void InternalDebugRenderer::DrawEllipseLocal(glm::vec2 const& size, std::vector<glm::vec3>& localPoints,
		std::vector<std::pair<uint32_t, uint32_t>>& lineIndices, uint32_t segments)
	{
		float const delta{ glm::two_pi<float>() / static_cast<float>(segments) };

		// Define unit vectors along the X and Y axis in local space
		glm::vec3 constexpr localV1{ 1.0f, 0.0f, 0.0f };
		glm::vec3 constexpr localV2{ 0.0f, 1.0f, 0.0f };

		localPoints.reserve(segments);
		for (uint32_t i{ 0 }; i < segments; ++i)
		{
			float const angle{ i * delta };

			glm::vec3 const point{ (size.x * glm::cos(angle) * localV1 + size.y * glm::sin(angle) * localV2) };
			localPoints.emplace_back(point);
		}

		lineIndices.reserve(segments);
		for (uint32_t i{ 0 }; i < segments; ++i)
		{
			uint32_t const nextIndex{ (i + 1) % segments };
			lineIndices.emplace_back(i, nextIndex);
		}
	}
}
