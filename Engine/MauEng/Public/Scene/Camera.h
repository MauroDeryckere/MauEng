#ifndef MAUENG_CAMERA_H
#define MAUENG_CAMERA_H

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace MauEng
{
	class Camera
	{
	public:
		explicit Camera(glm::vec3 const& pos, float fov = 60.f, float aspect = 19.f / 6.f, float near = .1f, float far = 100.f);
		Camera() = default;
		~Camera() = default;

		void Update() noexcept;

		void Focus(glm::vec3 const& position) noexcept;

		void Translate(glm::vec3 const& delta) noexcept;
		void RotateX(float amountDegrees) noexcept;
		void RotateY(float amountDegrees) noexcept;

		[[nodiscard]] glm::mat4 const& GetViewMatrix() const noexcept { return m_ViewMatrix; }
		[[nodiscard]] glm::mat4 const& GetProjectionMatrix() const noexcept { return m_ProjectionMatrix; }
		[[nodiscard]] glm::vec3 const& GetPosition() const noexcept { return m_Position; }

		void SetPosition(glm::vec3 const& pos) noexcept;
		void SetFOV(float newFov) noexcept;
		void SetAspectRatio(float newAspect) noexcept;

		void SetNear(float newNear) noexcept;
		void SetFar(float newFar) noexcept;

		enum class ToneMapper : uint32_t
		{
			ACESFilm,
			COUNT
		};

		void SetToneMapper(ToneMapper mapper) noexcept;
		[[nodiscard]] ToneMapper GetToneMapper() const noexcept { return m_ToneMapper; }

		void EnableExposure() noexcept { m_EnableExposure = true; }
		void DisableExposure() noexcept { m_EnableExposure = false; }
		[[nodiscard]] bool IsExposureEnabled() const noexcept { return m_EnableExposure; }

		void SetISO(float iso) noexcept { m_ISO = iso; }
		void SetAperture(float aperture) noexcept { m_Aperture = aperture; }
		void SetShutterSpeed(float shutterSpeed) noexcept { m_ShutterSpeed = shutterSpeed; }
		void SetExposureOverride(float exposure = 0.f) noexcept { m_ExposureOverride = exposure; }

		[[nodiscard]] float GetISO() const noexcept { return m_ISO ; }
		[[nodiscard]] float GetAperture() const noexcept { return m_Aperture; }
		[[nodiscard]] float GetShutterSpeed() const noexcept { return m_ShutterSpeed; }
		[[nodiscard]] float GetExposureOverride() const noexcept { return m_ExposureOverride; }

		void SetCamSettingsSunny16() noexcept;
		void SetCamSettingsIndoor() noexcept;

		Camera(Camera const&) = default;
		Camera(Camera&&) = default;
		Camera& operator=(Camera const&) = default;
		Camera& operator=(Camera&&) = default;

	private:
		glm::vec3 m_Position{ 0, 0, 0 };
		glm::quat m_Rotation{ 1, 0, 0, 0 };

		glm::vec3 m_Forward{ 0, 0, -1 };
		glm::vec3 m_Right{ 1, 0, 0 };
		glm::vec3 m_Up{ 0, 1, 0 };

		float m_Fov{ 60.f };
		float m_AspectRatio{ 16.f / 9.f };
		float m_NearPlane{ .1f };
		float m_FarPlane{ 100 };

		glm::mat4 m_ViewMatrix{};
		glm::mat4 m_ProjectionMatrix{};

		float m_Pitch{ 0.0f };
		float m_Yaw{ -90.0f };

		float m_MinPitch{ -89.f };
		float m_MaxPitch{ 89.f };

		// Should the camera be updated next time the update is called
		bool m_IsDirty{ false };

		// Auto exposure is not implemented yet
		bool m_IsAutoExposure{ false };
		bool m_EnableExposure{ true };

		ToneMapper m_ToneMapper{ ToneMapper::ACESFilm };

		float m_Aperture{ 1.4f };
		float m_ISO{ 1600.0f };
		float m_ShutterSpeed{ 1.0f / 60.f };
		float m_ExposureOverride{ 0.f };

		void UpdateViewMatrix() noexcept;
		void UpdateProjectionMatrix() noexcept;

		void UpdateDirectionFromEuler() noexcept;
	};
}

#endif