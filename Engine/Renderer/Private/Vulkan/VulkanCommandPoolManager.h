#ifndef MAUREN_VULKANCOMMANDPOOLMANAGER_H
#define MAUREN_VULKANCOMMANDPOOLMANAGER_H

#include "RendererPCH.h"

namespace MauRen
{
	// Only supports a single command pool for now, may be expanded to support more if necessary or simply create multiple "managers" and support initializing it with different parameters
	class VulkanCommandPoolManager final
	{
	public:
		VulkanCommandPoolManager() = default;
		~VulkanCommandPoolManager() = default;

		// Initializes the command pool
		void Initialize();

		// Destroys the command pool
		void Destroy();

		// Currently just creates the command buffers for all frames
		void CreateCommandBuffers();
		[[nodiscard]] VkCommandPool GetCommandPool(uint32_t index) noexcept;
		[[nodiscard]] VkCommandBuffer GetCommandBuffer(uint32_t index) const noexcept;
		[[nodiscard]] VkCommandBuffer& GetCommandBuffer(uint32_t index) noexcept;

		[[nodiscard]] VkCommandBuffer BeginSingleTimeCommands() const;
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;

		VulkanCommandPoolManager(VulkanCommandPoolManager const&) = delete;
		VulkanCommandPoolManager(VulkanCommandPoolManager&&) = delete;
		VulkanCommandPoolManager& operator=(VulkanCommandPoolManager const&) = delete;
		VulkanCommandPoolManager& operator=(VulkanCommandPoolManager&&) = delete;

	private:
		std::vector<VkCommandPool> m_CommandPools;
		VkCommandPool m_SingleTimeCommandPool;

		// Are automatically freed when their pool is destroyed
		std::vector<VkCommandBuffer> m_CommandBuffers{};

		void CreateCommandPool();
	};
}

#endif