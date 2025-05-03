#include "VulkanGraphicsPipeline.h"

#include "VulkanUtils.h"

namespace MauRen
{
	void VulkanGraphicsPipeline::Initialize(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount)
	{
		CreateGraphicsPipeline(pSwapChainContext, descriptorSetLayout, descriptorSetLayoutCount);
		CreateDepthPrePassPipeline(pSwapChainContext, descriptorSetLayout, descriptorSetLayoutCount);
		CreateDebugGraphicsPipeline(pSwapChainContext, descriptorSetLayout, descriptorSetLayoutCount);
		CreateGBufferPipeline(pSwapChainContext, descriptorSetLayout, descriptorSetLayoutCount);
	}

	void VulkanGraphicsPipeline::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_GraphicsPipeline, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_PipelineLayout, nullptr);

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_DebugPipeline, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_DebugPipelineLayout, nullptr);

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_DepthPrePassPipeline, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_DepthPrePassPipelineLayout, nullptr);
	}

	void VulkanGraphicsPipeline::CreateGraphicsPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		auto const vertShaderCode{ ReadFile("Resources/Shaders/shader.vert.spv") };
		auto const fragShaderCode{ ReadFile("Resources/Shaders/shader.frag.spv") };

		// Shader modules are linked internally, the VkShaderModule is just a wrapper so we do not need to store it.
		VkShaderModule vertShaderModule{ CreateShaderModule(vertShaderCode) };
		VkShaderModule fragShaderModule{ CreateShaderModule(fragShaderCode) };

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

		std::array const shaderStages{ vertShaderStageInfo, fragShaderStageInfo };

		std::array const dynamicStates{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();


		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto const bindingDescription{ VulkanUtils::GetVertexBindingDescription() };
		auto const attributeDescriptions{ VulkanUtils::GetVertexAttributeDescriptions() };

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;


		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(pSwapChainContext->GetExtent().width);
		viewport.height = static_cast<float>(pSwapChainContext->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = pSwapChainContext->GetExtent();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;


		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = deviceContext->GetSampleCount();
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional


		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;


		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_FALSE; // Written in the depth prepass
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		//depthStencil.front = {}; // Optional
		//depthStencil.back = {}; // Optional

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = descriptorSetLayoutCount;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (vkCreatePipelineLayout(deviceContext->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout!");
		}

		VkFormat colorFormat = pSwapChainContext->GetColorImage().format;
		std::vector<VkFormat> formats;
		formats.emplace_back(colorFormat);

		VkPipelineRenderingCreateInfo renderingCreate{};
		renderingCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingCreate.colorAttachmentCount = 1;
		renderingCreate.pColorAttachmentFormats = formats.data();
		renderingCreate.depthAttachmentFormat = pSwapChainContext->GetDepthImage().format;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(std::size(shaderStages));
		pipelineInfo.pStages = shaderStages.data();

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.layout = m_PipelineLayout;

		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;
		pipelineInfo.pNext =  &renderingCreate;

		// Note: These values are only used if the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is also specified in the flags field of VkGraphicsPipelineCreateInfo.
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(deviceContext->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline!");
		}
		ME_ASSERT(m_GraphicsPipeline != VK_NULL_HANDLE);

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), fragShaderModule, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), vertShaderModule, nullptr);
	}

	void VulkanGraphicsPipeline::CreateDepthPrePassPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		auto const vertShaderCode{ ReadFile("Resources/Shaders/depthPrepass.vert.spv") };

		// Shader modules are linked internally, the VkShaderModule is just a wrapper so we do not need to store it.
		VkShaderModule vertShaderModule{ CreateShaderModule(vertShaderCode) };

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		std::array const shaderStages{ vertShaderStageInfo };

		std::array const dynamicStates{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();


		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto const bindingDescription{ VulkanUtils::GetVertexBindingDescription() };
		auto const attributeDescriptions{ VulkanUtils::GetVertexAttributeDescriptions() };

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;


		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(pSwapChainContext->GetExtent().width);
		viewport.height = static_cast<float>(pSwapChainContext->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = pSwapChainContext->GetExtent();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;


		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = deviceContext->GetSampleCount();
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// Disabled for prepass
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.attachmentCount = 0;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;


		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = descriptorSetLayoutCount;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (VK_SUCCESS != vkCreatePipelineLayout(deviceContext->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_DepthPrePassPipelineLayout))
		{
			throw std::runtime_error("Failed to create pipeline layout!");
		}

		VkFormat colorFormat = pSwapChainContext->GetColorImage().format;
		std::vector<VkFormat> formats;
		formats.emplace_back(colorFormat);

		VkPipelineRenderingCreateInfo renderingCreate{};
		renderingCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingCreate.colorAttachmentCount = 0;
		renderingCreate.pColorAttachmentFormats = nullptr;
		renderingCreate.depthAttachmentFormat = pSwapChainContext->GetDepthImage().format;
		
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(std::size(shaderStages));
		pipelineInfo.pStages = shaderStages.data();

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.layout = m_DepthPrePassPipelineLayout;

		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;
		pipelineInfo.pNext = &renderingCreate;

		// Note: These values are only used if the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is also specified in the flags field of VkGraphicsPipelineCreateInfo.
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (VK_SUCCESS != vkCreateGraphicsPipelines(deviceContext->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_DepthPrePassPipeline))
		{
			throw std::runtime_error("Failed to create graphics pipeline!");
		}

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), vertShaderModule, nullptr);
	}

	void VulkanGraphicsPipeline::CreateDebugGraphicsPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		auto const debugVertShaderCode{ ReadFile("Resources/Shaders/debug_shader.vert.spv") };
		auto const debugFragShaderCode{ ReadFile("Resources/Shaders/debug_shader.frag.spv") };

		VkShaderModule debugVertShaderModule{ CreateShaderModule(debugVertShaderCode) };
		VkShaderModule debugFragShaderModule{ CreateShaderModule(debugFragShaderCode) };

		VkPipelineShaderStageCreateInfo debugVertShaderStageInfo{};
		debugVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		debugVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		debugVertShaderStageInfo.module = debugVertShaderModule;
		debugVertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo debugFragShaderStageInfo{};
		debugFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		debugFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		debugFragShaderStageInfo.module = debugFragShaderModule;
		debugFragShaderStageInfo.pName = "main";

		std::array const debugShaderStages{ debugVertShaderStageInfo, debugFragShaderStageInfo };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = descriptorSetLayoutCount;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;


		if (vkCreatePipelineLayout(deviceContext->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_DebugPipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create debug pipeline layout!");
		}


		VkFormat colorFormat = pSwapChainContext->GetColorImage().format;
		std::vector<VkFormat> formats;
		formats.emplace_back(colorFormat);

		VkPipelineRenderingCreateInfo renderingCreate{};
		renderingCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingCreate.colorAttachmentCount = 1;
		renderingCreate.pColorAttachmentFormats = formats.data();
		renderingCreate.depthAttachmentFormat = pSwapChainContext->GetDepthImage().format;
		
		VkPipelineRasterizationStateCreateInfo debugRasterizer{};
		debugRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		debugRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		debugRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		debugRasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		debugRasterizer.lineWidth = 1.0f;
		debugRasterizer.rasterizerDiscardEnable = VK_FALSE;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(pSwapChainContext->GetExtent().width);
		viewport.height = static_cast<float>(pSwapChainContext->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = pSwapChainContext->GetExtent();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional

		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional


		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
											  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.logicOp = VK_LOGIC_OP_COPY;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &colorBlendAttachment;
		colorBlendState.blendConstants[0] = 0.0f;
		colorBlendState.blendConstants[1] = 0.0f;
		colorBlendState.blendConstants[2] = 0.0f;
		colorBlendState.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();


		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto const bindingDescription{ VulkanUtils::GetDebugVertexBindingDescription() };
		auto const attributeDescriptions{ VulkanUtils::GetDebugVertexAttributeDescriptions() };

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = deviceContext->GetSampleCount();
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkGraphicsPipelineCreateInfo debugPipelineInfo{};
		debugPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		debugPipelineInfo.stageCount = static_cast<uint32_t>(std::size(debugShaderStages));
		debugPipelineInfo.pStages = debugShaderStages.data();
		debugPipelineInfo.pVertexInputState = &vertexInputInfo;
		debugPipelineInfo.pRasterizationState = &debugRasterizer;
		debugPipelineInfo.layout = m_DebugPipelineLayout;
		debugPipelineInfo.pViewportState = &viewportState;

		debugPipelineInfo.renderPass = VK_NULL_HANDLE;

		debugPipelineInfo.pDepthStencilState = &depthStencil;
		debugPipelineInfo.pColorBlendState = &colorBlendState;
		debugPipelineInfo.pDynamicState = &dynamicState;
		debugPipelineInfo.pInputAssemblyState = &inputAssemblyState;
		debugPipelineInfo.pMultisampleState = &multisampling;

		debugPipelineInfo.pNext = &renderingCreate;

		if (vkCreateGraphicsPipelines(deviceContext->GetLogicalDevice(), VK_NULL_HANDLE, 1, &debugPipelineInfo, nullptr, &m_DebugPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create debug graphics pipeline!");
		}

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), debugFragShaderModule, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), debugVertShaderModule, nullptr);
	}

	void VulkanGraphicsPipeline::CreateGBufferPipeline(VulkanSwapchainContext* pSwapChainContext, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorSetLayoutCount)
	{

	}

	std::vector<char> VulkanGraphicsPipeline::ReadFile(std::filesystem::path const& filepath)
	{
		ME_RENDERER_ASSERT(std::filesystem::exists(filepath));

		std::ifstream file(filepath, std::ios::binary | std::ios::in );
		if (!file)
		{
			throw std::runtime_error("failed to open file!");
		}

		auto const fileSize{ std::filesystem::file_size(filepath) };

		std::vector<char> buffer(fileSize);
		file.read(buffer.data(), fileSize);

		return buffer;
	}

	VkShaderModule VulkanGraphicsPipeline::CreateShaderModule(std::vector<char> const& code) const
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(deviceContext->GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}
}
