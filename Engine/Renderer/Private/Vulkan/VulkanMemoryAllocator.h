#ifndef MAUREN_VULKANMEMORYALLOCATOR_H
#define MAUREN_VULKANMEMORYALLOCATOR_H

#include "RendererPCH.h"

namespace MauRen
{
	class VulkanInstanceContext;

	class VulkanMemoryAllocator final : public MauCor::Singleton<VulkanMemoryAllocator>
	{
	public:
		void Initialize(VulkanInstanceContext const& instanceContext);
		void Destroy();

		[[nodiscard]] VmaAllocator GetAllocator() const noexcept;

		[[nodiscard]] static VmaMemoryUsage GetMemoryUsageFromVkProperties(VkMemoryPropertyFlags properties) noexcept;

		VulkanMemoryAllocator(VulkanMemoryAllocator const&) = delete;
		VulkanMemoryAllocator(VulkanMemoryAllocator&&) = delete;
		VulkanMemoryAllocator& operator=(VulkanMemoryAllocator const&) = delete;
		VulkanMemoryAllocator& operator=(VulkanMemoryAllocator const&&) = delete;
	private:
		friend class MauCor::Singleton<VulkanMemoryAllocator>;
		VulkanMemoryAllocator() = default;
		virtual ~VulkanMemoryAllocator() override = default;

		VmaAllocator m_Allocator{ nullptr };

	};
}

#endif