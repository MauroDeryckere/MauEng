#include "VulkanGraphicsPipeline.h"

namespace MauRen
{
	VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDeviceContext* pDeviceContex) :
		m_pDeviceContext{ pDeviceContex }
	{
		auto const vertShaderCode{ ReadFile("shaders/shader.vert.spv") };
		auto const fragShaderCode { ReadFile("shaders/shader.vert.spv") };

		// Shader modules are linked internally, the VkShaderModule is just a wrapper so we do not need to store it.
		VkShaderModule const vertShaderModule{ CreateShaderModule(vertShaderCode) };
		VkShaderModule const fragShaderModule{ CreateShaderModule(fragShaderCode) };

		vkDestroyShaderModule(m_pDeviceContext->GetLogicalDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(m_pDeviceContext->GetLogicalDevice(), vertShaderModule, nullptr);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo const shaderStages[] { vertShaderStageInfo, fragShaderStageInfo };
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{

	}
}
