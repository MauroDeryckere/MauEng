#include "Scene/CameraManager.h"

namespace MauEng
{
	CameraManager::CameraManager()
	{
		SetActiveCamera(CreateCamera());
	}

	CameraID CameraManager::CreateCamera(Camera const& camera) noexcept
	{
		m_Cameras[m_NextID] = camera;
		return m_NextID++;
	}

	bool CameraManager::SetActiveCamera(CameraID cameraID) noexcept
	{
		if (m_Cameras.contains(cameraID))
		{
			m_ActiveCamera = cameraID;
			return true;
		}
		ME_LOG_WARN(MauCor::ELogCategory::Engine, "Trying to set camera active but cameraID does not exist: {} ", cameraID);
		return false;
	}

	bool CameraManager::RemoveCamera(CameraID cameraID) noexcept
	{
		auto const it{ m_Cameras.find(cameraID) };
		if (it == m_Cameras.end())
		{
			ME_LOG_WARN(MauCor::ELogCategory::Engine, "Trying to remove a camera but cameraID does not exist: {}", cameraID);
			return false;
		}

		m_Cameras.erase(it);

		if (m_ActiveCamera == cameraID)
		{
			ME_LOG_WARN(MauCor::ELogCategory::Engine, "Removed camera was the active camera, trying to set a new active camera; {}", cameraID);
			if (!m_Cameras.empty())
			{
				ME_LOG_WARN(MauCor::ELogCategory::Engine, "Setting active camera to first available camera: {}", m_Cameras.begin()->first);
				m_ActiveCamera = m_Cameras.begin()->first;
				return true;
			}

			ME_LOG_ERROR(MauCor::ELogCategory::Engine, "Can not set an active camera, all cameras were removed");
			m_ActiveCamera = 0;
			return false;
		}

		return true;
	}

	Camera* const CameraManager::GetActiveCamera() noexcept
	{
		return GetCamera(m_ActiveCamera);
	}

	Camera const* const CameraManager::GetActiveCamera() const noexcept
	{
		return GetCamera(m_ActiveCamera);
	}

	Camera* const CameraManager::GetCamera(CameraID cameraID) noexcept
	{
		auto const it{ m_Cameras.find(cameraID) };
		if (it == std::end(m_Cameras))
		{
			ME_LOG_WARN(MauCor::ELogCategory::Engine, "Trying to get a camera for a given ID but the camera doesn't exist: {}", cameraID);
			return nullptr;
		}

		return &it->second;
	}

	Camera const* const CameraManager::GetCamera(CameraID cameraID) const noexcept
	{
		auto const it{ m_Cameras.find(cameraID) };
		if (it == std::end(m_Cameras))
		{
			ME_LOG_WARN(MauCor::ELogCategory::Engine, "Trying to get a camera for a given ID but the camera doesn't exist: {}", cameraID);
			return nullptr;
		}

		return &it->second;
	}

	void CameraManager::Tick() noexcept
	{
		m_Cameras[m_ActiveCamera].Update();
	}
}
