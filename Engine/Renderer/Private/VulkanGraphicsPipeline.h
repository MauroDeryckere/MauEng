#ifndef MAUREN_VULKANGRAPHICSPIPELINE_H
#define MAUREN_VULKANGRAPHICSPIPELINE_H

#include "RendererPCH.h"
#include "VulkanDeviceContext.h"
#include "VulkanSwapchainContext.h"
#include <filesystem>
#include <fstream>

namespace MauRen
{
	class VulkanGraphicsPipeline final
	{
	public:
		VulkanGraphicsPipeline(VulkanDeviceContext* pDeviceContext, VulkanSwapchainContext* pSwapChainContext);
		~VulkanGraphicsPipeline();

		[[nodiscard]] VkRenderPass GetRenderPass() const noexcept { return m_RenderPass; }
		[[nodiscard]] VkPipeline GetPipeline() const noexcept { return m_GraphicsPipeline; }

		VulkanGraphicsPipeline(VulkanGraphicsPipeline const&) = delete;
		VulkanGraphicsPipeline(VulkanGraphicsPipeline&&) = delete;
		VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline const&) = delete;
		VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&&) = delete;

	private:
		VulkanDeviceContext* m_pDeviceContext;
		VkRenderPass m_RenderPass;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;

		void CreateRenderPass(VulkanSwapchainContext* pSwapChainContext);
		void CreateGraphicsPipeline(VulkanSwapchainContext* pSwapChainContext);

		static std::vector<char> ReadFile(std::filesystem::path const& filepath);
		[[nodiscard]] VkShaderModule CreateShaderModule(std::vector<char> const& code) const;
	};
}

#endif // MAUREN_VULKANGRAPHICSPIPELINE_H