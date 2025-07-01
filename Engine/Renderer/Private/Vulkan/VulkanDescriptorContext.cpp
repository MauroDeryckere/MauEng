#include "VulkanDescriptorContext.h"

namespace MauRen
{
	void VulkanDescriptorContext::BindTexture(uint32_t destLocation, VkImageView imageView, VkImageLayout imageLayout)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = imageLayout;
		imageInfo.imageView = imageView;
		imageInfo.sampler = VK_NULL_HANDLE;

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_DescriptorSetUpdates[i].emplace_back(
				DescriptorSetUpdate::CreateImageUpdate(
					m_DescriptorSets[i], 
					TEXTURE_BINDING_SLOT, 
					destLocation, 
					VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 
					{ imageInfo }
				)
			);
		}
	}

	void VulkanDescriptorContext::BindMaterialBuffer(VkDescriptorBufferInfo bufferInfo, uint32_t frame)
	{
		m_DescriptorSetUpdates[frame].emplace_back(
			DescriptorSetUpdate::CreateBufferUpdate(
				m_DescriptorSets[frame],
				MATERIAL_DATA_BINDING_SLOT,
				0,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				{ bufferInfo }
			)
		);
	}

	void VulkanDescriptorContext::BindLightBuffer(VkDescriptorBufferInfo bufferInfo, uint32_t frame)
	{
		m_DescriptorSetUpdates[frame].emplace_back(
			DescriptorSetUpdate::CreateBufferUpdate(
				m_DescriptorSets[frame],
				LIGHT_BUFFER_SLOT,
				0,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				{ bufferInfo }
			)
		);
	}

	void VulkanDescriptorContext::BindShadowMap(uint32_t destLocation, VkImageView imageView, VkImageLayout imageLayout)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = imageLayout;
		imageInfo.imageView = imageView;
		imageInfo.sampler = VK_NULL_HANDLE;

		ME_ASSERT(destLocation < MAX_SHADOW_MAPS);

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_DescriptorSetUpdates[i].emplace_back(
				DescriptorSetUpdate::CreateImageUpdate(
					m_DescriptorSets[i],
					SHADOW_MAPS_SLOT,
					destLocation,
					VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
					{ imageInfo }
				)
			);
		}
	}

	void VulkanDescriptorContext::BindShadowMapSampler(VkSampler sampler)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = sampler;

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_DescriptorSetUpdates[i].emplace_back(
				DescriptorSetUpdate::CreateImageUpdate(
					m_DescriptorSets[i],
					SHADOW_MAP_SAMPLER,
					0,
					VK_DESCRIPTOR_TYPE_SAMPLER,
					{ imageInfo }
				)
			);
		}
	}

	void VulkanDescriptorContext::BindMeshInstanceDataBuffer(VkDescriptorBufferInfo bufferInfo, uint32_t frame)
	{
		m_DescriptorSetUpdates[frame].emplace_back(
			DescriptorSetUpdate::CreateBufferUpdate(
				m_DescriptorSets[frame],
				MESH_INSTANCE_DATA_BINDING_SLOT,
				0,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				{ bufferInfo }
			)
		);
	}

	void VulkanDescriptorContext::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = UBO_BINDING_SLOT;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		// This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation, for example.
		// Our MVP transformation is in a single uniform buffer object, so we're using a descriptorCount of 1.
		uboLayoutBinding.descriptorCount = 1;

		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		// Binding for global sampler
		VkDescriptorSetLayoutBinding samplerBinding{};
		samplerBinding.binding = SAMPLER_BINDING_SLOT;
		samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerBinding.descriptorCount = 1;
		samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding bindlessTextureBinding{};
		bindlessTextureBinding.binding = TEXTURE_BINDING_SLOT;
		bindlessTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		bindlessTextureBinding.descriptorCount = MAX_TEXTURES;
		bindlessTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Binding for material data
		VkDescriptorSetLayoutBinding materialDataBinding{};
		materialDataBinding.binding = MATERIAL_DATA_BINDING_SLOT;
		materialDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		materialDataBinding.descriptorCount = 1;
		materialDataBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		materialDataBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding meshDataBinding{};
		meshDataBinding.binding = MESH_DATA_BINDING_SLOT;
		meshDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		meshDataBinding.descriptorCount = 1;
		meshDataBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		meshDataBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding meshInstanceDataBinding{};
		meshInstanceDataBinding.binding = MESH_INSTANCE_DATA_BINDING_SLOT;
		meshInstanceDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		meshInstanceDataBinding.descriptorCount = 1;
		meshInstanceDataBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		meshInstanceDataBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding GBufferColorBinding{};
		GBufferColorBinding.binding = GBUFFER_COLOR_SLOT;
		GBufferColorBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		GBufferColorBinding.descriptorCount = 1;
		GBufferColorBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		GBufferColorBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding GBufferNormalBinding{};
		GBufferNormalBinding.binding = GBUFFER_NORMAL_SLOT;
		GBufferNormalBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		GBufferNormalBinding.descriptorCount = 1;
		GBufferNormalBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		GBufferNormalBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding GBufferMetalRoughBinding{};
		GBufferMetalRoughBinding.binding = GBUFFER_METAL_SLOT;
		GBufferMetalRoughBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		GBufferMetalRoughBinding.descriptorCount = 1;
		GBufferMetalRoughBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		GBufferMetalRoughBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding GBufferDepthBinding{};
		GBufferDepthBinding.binding = GBUFFER_DEPTH_SLOT;
		GBufferDepthBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		GBufferDepthBinding.descriptorCount = 1;
		GBufferDepthBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		GBufferDepthBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding HDRIColorBinding{};
		HDRIColorBinding.binding = HDRI_COLOR_SLOT;
		HDRIColorBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		HDRIColorBinding.descriptorCount = 1;
		HDRIColorBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		HDRIColorBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding ShadowMapsBinding{};
		ShadowMapsBinding.binding = SHADOW_MAPS_SLOT;
		ShadowMapsBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		ShadowMapsBinding.descriptorCount = MAX_SHADOW_MAPS;
		ShadowMapsBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		ShadowMapsBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding LightBufferBinding{};
		LightBufferBinding.binding = LIGHT_BUFFER_SLOT;
		LightBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		LightBufferBinding.descriptorCount = 1;
		LightBufferBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
		LightBufferBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding shadowMapSamplerBinding{};
		shadowMapSamplerBinding.binding = SHADOW_MAP_SAMPLER;
		shadowMapSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		shadowMapSamplerBinding.descriptorCount = 1;
		shadowMapSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		shadowMapSamplerBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding camSettingsUBO{};
		camSettingsUBO.binding = CAM_SETTINGS_SLOT;
		camSettingsUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		camSettingsUBO.descriptorCount = 1;
		camSettingsUBO.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array const bindings {
			uboLayoutBinding,
			samplerBinding,
			bindlessTextureBinding,
			materialDataBinding,
			meshDataBinding,
			meshInstanceDataBinding,
			GBufferColorBinding,
			GBufferNormalBinding,
			GBufferMetalRoughBinding,
			GBufferDepthBinding,
			HDRIColorBinding,
			ShadowMapsBinding,
			LightBufferBinding,
			shadowMapSamplerBinding,
			camSettingsUBO
		};

		// Variable coutn adds more complexity and we do not need it currentl
		// Flags for the binding - only use valid flags for image samplers
		VkDescriptorBindingFlagsEXT bindingFlags[bindings.size()]{
			0,
			0,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
			//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, // Flags for bindless textures
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
			//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, // Flags for shadow maps
			0,
			0,
			0
		};
		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
		bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		bindingFlagsInfo.pBindingFlags = bindingFlags;

		VkDescriptorSetLayoutCreateFlags descLayoutFlags{};
		descLayoutFlags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = &bindingFlagsInfo;
		layoutInfo.flags = descLayoutFlags;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		if (vkCreateDescriptorSetLayout(deviceContext->GetLogicalDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
	}

	void VulkanDescriptorContext::CreateDescriptorPool()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		{
			std::array<VkDescriptorPoolSize, 15> poolSizes{};
			poolSizes[UBO_BINDING_SLOT].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[UBO_BINDING_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[SAMPLER_BINDING_SLOT].type = VK_DESCRIPTOR_TYPE_SAMPLER;
			poolSizes[SAMPLER_BINDING_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[TEXTURE_BINDING_SLOT].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			poolSizes[TEXTURE_BINDING_SLOT].descriptorCount = static_cast<uint32_t>(MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT);

			poolSizes[MATERIAL_DATA_BINDING_SLOT].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSizes[MATERIAL_DATA_BINDING_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[MESH_DATA_BINDING_SLOT].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSizes[MESH_DATA_BINDING_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[MESH_INSTANCE_DATA_BINDING_SLOT].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSizes[MESH_INSTANCE_DATA_BINDING_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[GBUFFER_COLOR_SLOT].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			poolSizes[GBUFFER_COLOR_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[GBUFFER_NORMAL_SLOT].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			poolSizes[GBUFFER_NORMAL_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[GBUFFER_METAL_SLOT].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			poolSizes[GBUFFER_METAL_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[GBUFFER_DEPTH_SLOT].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			poolSizes[GBUFFER_DEPTH_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[HDRI_COLOR_SLOT].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			poolSizes[HDRI_COLOR_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[SHADOW_MAPS_SLOT].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			poolSizes[SHADOW_MAPS_SLOT].descriptorCount = static_cast<uint32_t>(MAX_SHADOW_MAPS * MAX_FRAMES_IN_FLIGHT);

			poolSizes[LIGHT_BUFFER_SLOT].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSizes[LIGHT_BUFFER_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[SHADOW_MAP_SAMPLER].type = VK_DESCRIPTOR_TYPE_SAMPLER;
			poolSizes[SHADOW_MAP_SAMPLER].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			poolSizes[CAM_SETTINGS_SLOT].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[CAM_SETTINGS_SLOT].descriptorCount = static_cast<uint32_t>(1 * MAX_FRAMES_IN_FLIGHT);

			// + 5 == hdri, depth, metal, normal, color
			if (MAX_TEXTURES + MAX_SHADOW_MAPS + 5 > deviceContext->GetMaxSampledImages())
			{
				throw std::runtime_error("Max textures is bigger than device limitations");
			}

			if ([&]() -> uint32_t
				{
					uint32_t tot{ 0 };
					for (auto&& s : poolSizes)
					{
						tot += s.descriptorCount;
					}
					return tot;

				}() > deviceContext->GetMaxDescriptorSets())
			{
				throw std::runtime_error("descriptor set count is bigger than device limitations");
			}

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

			if (vkCreateDescriptorPool(deviceContext->GetLogicalDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {

				throw std::runtime_error("Failed to create descriptor pool!");
			}
		}

#pragma region ImGUI_Pool
		{
			VkDescriptorPoolSize constexpr POOL_SIZES[]
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			poolInfo.maxSets = 1000 * std::size(POOL_SIZES);
			poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(POOL_SIZES));
			poolInfo.pPoolSizes = POOL_SIZES;

			if (VK_SUCCESS != vkCreateDescriptorPool(deviceContext->GetLogicalDevice(), &poolInfo, nullptr, &m_ImGUIPool))
			{
				throw std::runtime_error("Failed to create ImGui descriptor pool!");
			}
		}
#pragma endregion
	}

	void VulkanDescriptorContext::CreateDescriptorSets(
		std::vector<VulkanBuffer> const& bufferInfoBuffers, VkDeviceSize offset, VkDeviceSize range, VkSampler sampler,
		std::vector<VulkanBuffer> const& bufferInfoBuffersCamSettings, VkDeviceSize offsetCamSettings, VkDeviceSize rangeCamSettings)
	{
		std::vector<VkDescriptorSetLayout> const layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		if (vkAllocateDescriptorSets(deviceContext->GetLogicalDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkWriteDescriptorSet descriptorWrites[3] = {};

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = bufferInfoBuffers[i].buffer;
			bufferInfo.offset = offset;
			bufferInfo.range = range;

			// Descriptor write for uniform buffer (binding 0)
			descriptorWrites[0] = {
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				m_DescriptorSets[i],
				UBO_BINDING_SLOT,  // Binding for the uniform buffer
				0,
				1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				nullptr,
				&bufferInfo,
				nullptr
				};

			VkDescriptorImageInfo samplerInfo{};
			samplerInfo.sampler = sampler;
			samplerInfo.imageView = VK_NULL_HANDLE;
			samplerInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			descriptorWrites[1] = {
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				m_DescriptorSets[i],
				SAMPLER_BINDING_SLOT,
				0,
				1,
				VK_DESCRIPTOR_TYPE_SAMPLER,
				&samplerInfo,
				nullptr,
				nullptr
			};

			VkDescriptorBufferInfo bufferInfoCamSettings{};
			bufferInfoCamSettings.buffer = bufferInfoBuffersCamSettings[i].buffer;
			bufferInfoCamSettings.offset = offsetCamSettings;
			bufferInfoCamSettings.range = rangeCamSettings;

			descriptorWrites[2] = {
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				m_DescriptorSets[i],
				CAM_SETTINGS_SLOT,
				0,
				1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				nullptr,
				&bufferInfoCamSettings,
				nullptr
			};

			vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), 3, descriptorWrites, 0, nullptr);
		}
	}

	void VulkanDescriptorContext::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_ImGUIPool, nullptr);

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_DescriptorPool, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_DescriptorSetLayout, nullptr);
	}

	void VulkanDescriptorContext::ProcessDescriptorUpdateQueue(uint32_t frame) noexcept
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		auto& updates{ m_DescriptorSetUpdates[frame] };

		for (auto const& update : updates)
		{
			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = update.targetSet;
			write.dstBinding = update.dstBinding;
			write.dstArrayElement = update.dstArrayElement;
			write.descriptorType = update.descriptorType;

			switch (update.type)
			{
			case DescriptorSetUpdate::EType::Buffer:
				write.pBufferInfo = std::get<std::vector<VkDescriptorBufferInfo>>(update.descriptorData).data();
				write.descriptorCount = static_cast<uint32_t>(std::size(std::get<std::vector<VkDescriptorBufferInfo>>(update.descriptorData)));
				break;
			case DescriptorSetUpdate::EType::Image:
				write.pImageInfo = std::get<std::vector<VkDescriptorImageInfo>>(update.descriptorData).data();
				write.descriptorCount = static_cast<uint32_t>(std::size(std::get<std::vector<VkDescriptorImageInfo>>(update.descriptorData)));
				break;
			case DescriptorSetUpdate::EType::TexelBuffer:
				write.pTexelBufferView = std::get<std::vector<VkBufferView>>(update.descriptorData).data();
				write.descriptorCount = static_cast<uint32_t>(std::size(std::get<std::vector<VkBufferView>>(update.descriptorData)));
				break;
			}

			vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), 1, &write, 0, nullptr);
		}

		updates.clear();
	}
}
