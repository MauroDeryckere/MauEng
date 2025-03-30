#include "VulkanMeshManager.h"

#include "MeshInstance.h"
#include "VulkanDeviceContextManager.h"
namespace MauRen
{
	bool VulkanMeshManager::Initialize(VulkanCommandPoolManager const* CmdPoolManager)
	{
		m_CmdPoolManager = CmdPoolManager;
		return true;
	}

	bool VulkanMeshManager::Destroy()
	{
		for (auto& m : m_Meshes)
		{
			m.second.Destroy();
		}

		return true;
	}

	void VulkanMeshManager::LoadMesh(Mesh& mesh)
	{
		auto it{ m_Meshes.find(mesh.GetMeshID()) };
		if (it != end(m_Meshes))
		{
			return;
		}

		mesh.SetMeshID(m_NextID);
		m_Meshes.emplace(m_NextID, VulkanMesh{ *m_CmdPoolManager, mesh });
		m_NextID++;
	}

	const VulkanMesh& VulkanMeshManager::GetVulkanMesh(uint32_t meshID) const
	{
		auto it = m_Meshes.find(meshID);
		if (it != m_Meshes.end())
		{
			return it->second;
		}

		assert(false && "Mesh not found in VulkanMeshManager");

		throw std::runtime_error("");
	}

	void VulkanMeshManager::QueueDraw(MeshInstance const& instance)
	{
		m_MeshBatches[instance.GetMeshID()].emplace_back(instance);
	}

	void VulkanMeshManager::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets)
	{
		// Draw each batch for each mesh
		// TODO actually batch them

		for (auto const& [meshID, instances] : m_MeshBatches)
		{
			VulkanMesh const& mesh = m_Meshes.at(meshID);

			std::vector<glm::mat4> modelMatrices;
			for (auto const& instance : instances)
			{
				//static auto startTime{ std::chrono::high_resolution_clock::now() };

				//auto const currentTime{ std::chrono::high_resolution_clock::now() };
				//float const deltaTime{ std::chrono::duration<float>(currentTime - startTime).count() };
				//startTime = currentTime; // Update start time for the next frame

				//float rotationSpeed = glm::radians(90.0f); // 90 degrees per second

				//glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotationSpeed * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f));

				VulkanMesh::MeshPushConstant mPush{ mesh.GetPushConstant() };
				mPush.m_ModelMatrix = instance.GetModelMatrix();
				mPush.m_AlbedoTextureID = 0;
				vkCmdPushConstants(commandBuffer, layout,
									VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mesh.GetPushConstant()),
									&mPush);

				mesh.Draw(commandBuffer);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, setCount, pDescriptorSets, 0, nullptr);
				vkCmdDrawIndexed(commandBuffer, mesh.GetIndexCount(), 1, 0, 0, 0);
			}
		}
	}
}
