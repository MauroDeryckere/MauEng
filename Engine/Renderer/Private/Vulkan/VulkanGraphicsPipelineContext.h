#ifndef MAUREN_VULKANGRAPHICSPIPELINE_H
#define MAUREN_VULKANGRAPHICSPIPELINE_H

#include "RendererPCH.h"

#include "VulkanSwapchainContext.h"
#include <filesystem>
#include <fstream>

namespace MauRen
{
	class VulkanGraphicsPipelineContext final
	{
	public:
		VulkanGraphicsPipelineContext() = default;
		~VulkanGraphicsPipelineContext() = default;

		void Initialize(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount);
		void Destroy();

		[[nodiscard]] VkPipeline GetForwardPipeline() const noexcept { return m_ForwardPipeline; }
		[[nodiscard]] VkPipelineLayout GetForwardPipelineLayout() const noexcept { return m_ForwardPipelineLayout; }

		[[nodiscard]] VkPipeline GetDepthPrePassPipeline() const noexcept { return m_DepthPrePassPipeline; }
		[[nodiscard]] VkPipelineLayout GetDepthPrePassPipelineLayout() const noexcept { return m_DepthPrePassPipelineLayout; }

		[[nodiscard]] VkPipeline GetDebugPipeline() const noexcept { return m_DebugPipeline; }
		[[nodiscard]] VkPipelineLayout GetDebugPipelineLayout() const noexcept { return m_DebugPipelineLayout; }

		[[nodiscard]] VkPipeline GetGBufferPipeline() const noexcept { return m_GBufferPipeline; }
		[[nodiscard]] VkPipelineLayout GetGBufferPipelineLayout() const noexcept { return m_GBufferPipelineLayout; }

		VulkanGraphicsPipelineContext(VulkanGraphicsPipelineContext const&) = delete;
		VulkanGraphicsPipelineContext(VulkanGraphicsPipelineContext&&) = delete;
		VulkanGraphicsPipelineContext& operator=(VulkanGraphicsPipelineContext const&) = delete;
		VulkanGraphicsPipelineContext& operator=(VulkanGraphicsPipelineContext&&) = delete;

	private:
		VkPipelineLayout m_ForwardPipelineLayout{ VK_NULL_HANDLE };
		VkPipeline m_ForwardPipeline{ VK_NULL_HANDLE };

		VkPipelineLayout m_DepthPrePassPipelineLayout{ VK_NULL_HANDLE };
		VkPipeline m_DepthPrePassPipeline{ VK_NULL_HANDLE };

		VkPipelineLayout m_DebugPipelineLayout{ VK_NULL_HANDLE };
		VkPipeline m_DebugPipeline{ VK_NULL_HANDLE };

		VkPipelineLayout m_GBufferPipelineLayout{ VK_NULL_HANDLE };
		VkPipeline m_GBufferPipeline{ VK_NULL_HANDLE };

		VkPipelineLayout m_LightPassPipelineLayout{ VK_NULL_HANDLE };
		VkPipeline m_LightPassPipeline{ VK_NULL_HANDLE };

		void CreateForwardPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount);
		void CreateDepthPrePassPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount);
		void CreateDebugGraphicsPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount);
		void CreateGBufferPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount);
		void CreateLightPassPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount);

		static [[nodiscard]] std::vector<char> ReadFile(std::filesystem::path const& filepath);
		static [[nodiscard]] VkShaderModule CreateShaderModule(std::vector<char> const& code);
	};
}

#endif // MAUREN_VULKANGRAPHICSPIPELINE_H