#include "VulkanMemoryAllocator.h"
#include "VulkanDeviceContextManager.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace MauRen
{
	void VulkanMemoryAllocator::Initialize(VulkanInstanceContext const& instanceContext)
	{
		auto deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = deviceContext->GetPhysicalDevice();
		allocatorInfo.device = deviceContext->GetLogicalDevice();
		allocatorInfo.instance = instanceContext.GetInstance();

		VmaAllocator vmaAllocator;
		VkResult const result{ vmaCreateAllocator(&allocatorInfo, &vmaAllocator) };
		ME_ASSERT(result == VK_SUCCESS);

		m_Allocator = vmaAllocator;
	}

	void VulkanMemoryAllocator::Destroy()
	{
		vmaDestroyAllocator(m_Allocator);
	}

	VmaAllocator VulkanMemoryAllocator::GetAllocator() const noexcept
	{
		return m_Allocator;
	}
}
