#include "VulkanCommandPoolManager.h"

namespace MauRen
{
	void VulkanCommandPoolManager::Initialize()
	{
		CreateCommandPool();
	}

	void VulkanCommandPoolManager::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		for (auto pool : m_CommandPools)
		{
			VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), pool, nullptr);
		}

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_SingleTimeCommandPool, nullptr);
	}

	void VulkanCommandPoolManager::CreateCommandBuffers()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_CommandPools[i];
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			if (vkAllocateCommandBuffers(deviceContext->GetLogicalDevice(), &allocInfo, &m_CommandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to allocate command buffers!");
			}
		}
	}

	VkCommandPool VulkanCommandPoolManager::GetCommandPool(uint32_t index) noexcept
	{
		return m_CommandPools[index];
	}

	VkCommandBuffer VulkanCommandPoolManager::GetCommandBuffer(uint32_t index) const noexcept
	{
		assert(index < m_CommandBuffers.size());
		return m_CommandBuffers[index];
	}

	VkCommandBuffer& VulkanCommandPoolManager::GetCommandBuffer(uint32_t index) noexcept
	{
		assert(index < m_CommandBuffers.size());
		return m_CommandBuffers[index];
	}

	VkCommandBuffer VulkanCommandPoolManager::BeginSingleTimeCommands() const
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_SingleTimeCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer{};
		vkAllocateCommandBuffers(deviceContext->GetLogicalDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanCommandPoolManager::EndSingleTimeCommands(VkCommandBuffer commandBuffer) const
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(deviceContext->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(deviceContext->GetGraphicsQueue());

		vkFreeCommandBuffers(deviceContext->GetLogicalDevice(), m_SingleTimeCommandPool, 1, &commandBuffer);

		// TODO 
		// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete, instead of executing one at a time.
		// That may give the driver more opportunities to optimize.
	}

	void VulkanCommandPoolManager::CreateCommandPool()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		// Record commands for drawing, which is why we've chosen the graphics queue family
		QueueFamilyIndices const queueFamilyIndices{ deviceContext->FindQueueFamilies() };

		m_CommandPools.resize(MAX_FRAMES_IN_FLIGHT);
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = 0;
			poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				if (vkCreateCommandPool(deviceContext->GetLogicalDevice(), &poolInfo, nullptr, &m_CommandPools[i]) != VK_SUCCESS)
				{
					throw std::runtime_error("Failed to create command pool!");
				}
			}
		}

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		vkCreateCommandPool(deviceContext->GetLogicalDevice(), &poolInfo, nullptr, &m_SingleTimeCommandPool);
	}
}