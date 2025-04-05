#ifndef MAUENG_CAMERAMANAGER_H
#define MAUENG_CAMERAMANAGER_H

#include "Camera.h"

namespace MauEng
{

	class CameraManager final
	{
	public:
		CameraManager() = default;
		~CameraManager() = default;

		void Tick() noexcept;

		[[nodiscard]] Camera& GetActiveCamera() noexcept { return m_Camera; }
		[[nodiscard]] Camera const& GetActiveCamera() const noexcept { return m_Camera; }

		CameraManager(CameraManager const&) = delete;
		CameraManager(CameraManager&&) = delete;
		CameraManager& operator=(CameraManager const&) = delete;
		CameraManager& operator=(CameraManager&&) = delete;

	private:
		Camera m_Camera{ };

	};
}
		
#endif
