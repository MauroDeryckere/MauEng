#include "MeshInstance.h"

#include "Vulkan/VulkanMeshManager.h"

namespace MauRen
{
	MeshInstance::MeshInstance(Mesh const& mesh) :
		m_MeshID { mesh.GetMeshID() },
		m_MaterialID { mesh.GetMaterialID() }
	{
		ME_RENDERER_ASSERT(m_MeshID != UINT32_MAX);
		ME_RENDERER_ASSERT(m_MaterialID != UINT32_MAX);
	}

	void MeshInstance::Draw() const
	{
		VulkanMeshManager::GetInstance().QueueDraw(this);
	}

	void MeshInstance::Translate(glm::vec3 const& translation) noexcept
	{
		m_ModelMatrix = glm::translate(m_ModelMatrix, translation);
	}

	void MeshInstance::ResetTransformation() noexcept
	{
		m_ModelMatrix = glm::mat4{1.0f};
	}

	void MeshInstance::Rotate(MauCor::Rotator const& rotator) noexcept
	{
		m_ModelMatrix *= glm::toMat4(rotator.rotation);
	}

	void MeshInstance::Scale(glm::vec3 const& scale) noexcept
	{
		m_ModelMatrix = glm::scale(m_ModelMatrix, scale);
	}
}


