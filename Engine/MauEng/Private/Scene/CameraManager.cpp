#include "Scene/CameraManager.h"

namespace MauEng
{
	void CameraManager::Tick() noexcept
	{
		m_Camera.Update();
	}
}
