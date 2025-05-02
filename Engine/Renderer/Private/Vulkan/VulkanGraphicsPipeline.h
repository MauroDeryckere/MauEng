#ifndef MAUREN_VULKANGRAPHICSPIPELINE_H
#define MAUREN_VULKANGRAPHICSPIPELINE_H

#include "RendererPCH.h"

#include "VulkanSwapchainContext.h"
#include <filesystem>
#include <fstream>

namespace MauRen
{
	class VulkanGraphicsPipeline final
	{
	public:
		VulkanGraphicsPipeline() = default;
		~VulkanGraphicsPipeline() = default;

		void Initialize(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount);
		void Destroy();

		[[nodiscard]] VkPipeline GetPipeline() const noexcept { return m_GraphicsPipeline; }
		[[nodiscard]] VkPipelineLayout GetPipelineLayout() const noexcept { return m_PipelineLayout; }

		[[nodiscard]] VkPipeline GetDepthPrePassPipeline() const noexcept { return m_DepthPrePassPipeline; }
		[[nodiscard]] VkPipelineLayout GetDepthPrePassPipelineLayout() const noexcept { return m_DepthPrePassPipelineLayout; }

		[[nodiscard]] VkPipeline GetDebugPipeline() const noexcept { return m_DebugPipeline; }
		[[nodiscard]] VkPipelineLayout GetDebugPipelineLayout() const noexcept { return m_DebugPipelineLayout; }

		VulkanGraphicsPipeline(VulkanGraphicsPipeline const&) = delete;
		VulkanGraphicsPipeline(VulkanGraphicsPipeline&&) = delete;
		VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline const&) = delete;
		VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&&) = delete;

	private:
		VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };
		VkPipeline m_GraphicsPipeline{ VK_NULL_HANDLE };

		VkPipelineLayout m_DepthPrePassPipelineLayout{ VK_NULL_HANDLE };
		VkPipeline m_DepthPrePassPipeline{ VK_NULL_HANDLE };

		VkPipelineLayout m_DebugPipelineLayout{ VK_NULL_HANDLE };
		VkPipeline m_DebugPipeline{ VK_NULL_HANDLE };

		void CreateGraphicsPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount);
		void CreateDepthPrePassPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount);
		void CreateDebugGraphicsPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount);

		static std::vector<char> ReadFile(std::filesystem::path const& filepath);
		[[nodiscard]] VkShaderModule CreateShaderModule(std::vector<char> const& code) const;
	};
}

#endif // MAUREN_VULKANGRAPHICSPIPELINE_H