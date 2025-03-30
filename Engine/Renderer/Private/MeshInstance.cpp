#include "MeshInstance.h"

#include "Vulkan/VulkanMeshManager.h"

namespace MauRen
{
	MeshInstance::MeshInstance(Mesh const& mesh) :
	m_MeshID { mesh.GetMeshID() }
	{
		assert(m_MeshID != UINT32_MAX);
	}

	void MeshInstance::Draw() const
	{
		VulkanMeshManager::GetInstance().QueueDraw(*this);
	}

	void MeshInstance::Translate(glm::vec3 const& translation) noexcept
	{
		m_ModelMatrix = glm::translate(m_ModelMatrix, translation);
	}
}


