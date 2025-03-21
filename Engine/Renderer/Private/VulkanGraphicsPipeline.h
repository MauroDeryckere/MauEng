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

		VulkanGraphicsPipeline(VulkanGraphicsPipeline const&) = delete;
		VulkanGraphicsPipeline(VulkanGraphicsPipeline&&) = delete;
		VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline const&) = delete;
		VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&&) = delete;

	private:
		VulkanDeviceContext* m_pDeviceContext;
		VkPipelineLayout m_PipelineLayout;

		static std::vector<char> ReadFile(std::filesystem::path const& filepath)
		{
			assert(std::filesystem::exists(filepath));

			std::ifstream file(filepath, std::ios::ate | std::ios::binary);

			if (!file.is_open()) 
			{
				throw std::runtime_error("failed to open file!");
			}

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

			return buffer;
		}

		VkShaderModule CreateShaderModule(std::vector<char> const& code)
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

			VkShaderModule shaderModule;
			if (vkCreateShaderModule(m_pDeviceContext->GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create shader module!");
			}

			return shaderModule;
		}
	};
}

#endif // MAUREN_VULKANGRAPHICSPIPELINE_H