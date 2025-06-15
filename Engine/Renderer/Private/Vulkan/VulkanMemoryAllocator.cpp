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
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;

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

	VmaMemoryUsage VulkanMemoryAllocator::GetMemoryUsageFromVkProperties(VkMemoryPropertyFlags properties) noexcept
    {
        if ((properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) &&
            !(properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            return VMA_MEMORY_USAGE_GPU_ONLY;
        }
        else if ((properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            (properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) &&
            !(properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            return VMA_MEMORY_USAGE_CPU_ONLY;
        }
        else if ((properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            return VMA_MEMORY_USAGE_CPU_TO_GPU;
        }
        else if ((properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            (properties & VK_MEMORY_PROPERTY_HOST_CACHED_BIT))
        {
            return VMA_MEMORY_USAGE_GPU_TO_CPU;
        }

        // Fallback
        return VMA_MEMORY_USAGE_UNKNOWN;
    }
}
