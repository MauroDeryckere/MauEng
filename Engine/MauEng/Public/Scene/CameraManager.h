#ifndef MAUENG_CAMERAMANAGER_H
#define MAUENG_CAMERAMANAGER_H

#include "Camera.h"

namespace MauEng
{
	using CameraID = uint32_t;

	class CameraManager final
	{
	public:
		CameraManager();
		~CameraManager() = default;

		CameraID CreateCamera(Camera const& camera = Camera{}) noexcept;
		bool SetActiveCamera(CameraID cameraID) noexcept;
		bool RemoveCamera(CameraID cameraID) noexcept;

		// Nullptr if invalid
		[[nodiscard]] Camera* const GetActiveCamera() noexcept;
		// Nullptr if invalid
		[[nodiscard]] Camera const* const GetActiveCamera() const noexcept;

		// Nullptr if invalid
		[[nodiscard]] Camera* const GetCamera(CameraID cameraID) noexcept;
		// Nullptr if invalid
		[[nodiscard]] Camera const * const GetCamera(CameraID cameraID) const noexcept;

		[[nodiscard]] CameraID GetActiveCameraID() const noexcept { return m_ActiveCamera; }

		// Update active cam
		void Tick() noexcept;

		using iterator = std::unordered_map<CameraID, Camera>::iterator;
		using const_iterator = std::unordered_map<CameraID, Camera>::const_iterator;

		[[nodiscard]] iterator begin() noexcept { return m_Cameras.begin(); }
		[[nodiscard]] iterator end() noexcept { return m_Cameras.end(); }
		[[nodiscard]] const_iterator begin() const noexcept { return m_Cameras.begin(); }
		[[nodiscard]] const_iterator end() const noexcept { return m_Cameras.end(); }
		[[nodiscard]] const_iterator cbegin() const noexcept { return m_Cameras.cbegin(); }
		[[nodiscard]] const_iterator cend() const noexcept { return m_Cameras.cend(); }

		CameraManager(CameraManager const&) = delete;
		CameraManager(CameraManager&&) = delete;
		CameraManager& operator=(CameraManager const&) = delete;
		CameraManager& operator=(CameraManager&&) = delete;
	private:
		std::unordered_map<CameraID, Camera> m_Cameras{};

		CameraID m_ActiveCamera{ 0 };
		CameraID m_NextID{ 0 };
	};
}
		
#endif
