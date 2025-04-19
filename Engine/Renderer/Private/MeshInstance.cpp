#include "MeshInstance.h"

#include "Vulkan/VulkanMeshManager.h"

namespace MauRen
{
	void MeshInstance::Draw() const
	{
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


