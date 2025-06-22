#include "Scene/Camera.h"

#include "EnginePCH.h"

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
		SetCamSettingsSunny16();
	}

	void Camera::UpdateViewMatrix() noexcept
	{
		UpdateDirectionFromEuler();

		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Forward, glm::vec3{ 0.0f, 1.0f, 0.0f });
	}

	void Camera::UpdateProjectionMatrix() noexcept
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearPlane, m_FarPlane);
		m_ProjectionMatrix[1][1] *= -1;
	}

	void Camera::UpdateDirectionFromEuler() noexcept
	{
		float pitchRad = glm::radians(m_Pitch);
		float yawRad = glm::radians(m_Yaw);

		m_Forward = glm::normalize(glm::vec3{
			cos(pitchRad) * cos(yawRad),
			sin(pitchRad),
			cos(pitchRad) * sin(yawRad)
			});

		m_Right = glm::normalize(glm::cross(m_Forward, glm::vec3{ 0.0f, 1.0f, 0.0f }));
		m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
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

		m_Pitch = glm::degrees(asin(m_Forward.y));
		m_Yaw = glm::degrees(atan2(m_Forward.z, m_Forward.x));

		UpdateViewMatrix();
	}

	void Camera::Translate(glm::vec3 const& delta) noexcept
	{
		glm::vec3 const forwardMove{ m_Forward * delta.z };
		glm::vec3 const rightMove{ m_Right * delta.x };
		glm::vec3 const upMove{ m_Up * delta.y };

		m_Position += forwardMove + rightMove + upMove;

		m_IsDirty = true;
	}

	void Camera::RotateY(float amountDegrees) noexcept
	{
		m_Pitch = std::clamp(m_Pitch + amountDegrees, m_MinPitch, m_MaxPitch);
		m_IsDirty = true;
	}

	void Camera::RotateX(float amountDegrees) noexcept
	{
		m_Yaw += amountDegrees;
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

	void Camera::SetNear(float newNear) noexcept
	{
		m_NearPlane = newNear;
		m_IsDirty = true;
	}

	void Camera::SetFar(float newFar) noexcept
	{
		m_FarPlane = newFar;
		m_IsDirty = true;
	}

	void Camera::SetToneMapper(ToneMapper mapper) noexcept
	{
		ME_ASSERT(ToneMapper::COUNT != mapper);
		m_ToneMapper = mapper;
	}

	void Camera::SetCamSettingsSunny16() noexcept
	{
		m_Aperture = 5.0f;
		m_ISO = 100.0f;
		m_ShutterSpeed = 1.0f / 200.f;

		m_ExposureOverride = 0.f;

		m_EnableExposure = true;
	}

	void Camera::SetCamSettingsIndoor() noexcept
	{
		m_Aperture = 1.4f;
		m_ISO = 1600.0f;
		m_ShutterSpeed = 1.0f / 60.f;

		m_ExposureOverride = 0.f;

		m_EnableExposure = true;
	}
}
