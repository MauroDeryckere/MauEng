#include "Scene/Camera.h"

#include <algorithm>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include "glm/detail/type_quat.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"

namespace MauEng
{
	Camera::Camera(glm::vec3 const& pos, float fov, float aspect, float near, float far) :
		m_Position{ pos },
		m_Fov{ fov },
		m_AspectRatio{ aspect },
		m_NearPlane{ near },
		m_FarPlane{ far }
	{
		UpdateProjectionMatrix();
		UpdateViewMatrix();
	}

	void Camera::UpdateViewMatrix() noexcept
	{
		m_Right = glm::normalize(glm::cross(m_Forward, {0, 0, 1}));
		m_Up = glm::normalize(glm::cross(m_Right, m_Forward));

		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
	}

	void Camera::UpdateProjectionMatrix() noexcept
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearPlane, m_FarPlane);
		m_ProjectionMatrix[1][1] *= -1;
	}

	void Camera::Update() noexcept
	{
		if (!m_IsDirty)
		{
			return;
		}

		UpdateViewMatrix();
		UpdateProjectionMatrix();

		m_IsDirty = false;
	}

	void Camera::Focus(glm::vec3 const& position) noexcept
	{
		m_Forward = glm::normalize(position - m_Position);

		UpdateViewMatrix();
	}

	void Camera::Translate(glm::vec3 const& delta) noexcept
	{
		m_Position += m_Forward * delta.z;
		m_Position += m_Right * delta.x;
		m_Position += m_Up * delta.y;

		m_IsDirty = true;
	}

	void Camera::Rotate(float amountDegrees, glm::vec3 const& axis) noexcept
	{
		auto p = glm::degrees(asin(m_Forward.y));
		auto y = glm::degrees(atan2(m_Forward.z, m_Forward.x));
		
		std::cout << "Pitch: " << p << " Yaw: " << y << std::endl;

		auto axisCopy{ axis };
		float const yaw{ glm::degrees(atan2(m_Forward.z, m_Forward.x)) };

		if (p < 0)
		{
			axisCopy.x *= -1;
		}

		m_Forward = glm::normalize(glm::rotate(m_Forward, glm::radians(amountDegrees), axisCopy));



		m_IsDirty = true;
	}

	void Camera::SetPosition(glm::vec3 const& pos) noexcept
	{
		m_Position = pos;
		m_IsDirty = true;
	}

	void Camera::SetFOV(float newFov) noexcept
	{
		m_Fov = newFov;
		m_IsDirty = true;
	}

	void Camera::SetAspectRatio(float newAspect) noexcept
	{
		m_AspectRatio = newAspect;
		m_IsDirty = true;
	}
}
