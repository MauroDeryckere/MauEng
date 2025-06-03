#include "VulkanLightManager.h"

#include "VulkanMeshManager.h"
#include "../../../../MauEng/Public/ServiceLocator.h"
#include "../../MauEng/Public/Components/CLight.h"
#include "Assets/ImageLoader.h"
#include "Vulkan/VulkanCommandPoolManager.h"
#include "Vulkan/VulkanDescriptorContext.h"
#include "Vulkan/VulkanGraphicsPipelineContext.h"


namespace MauRen
{
	void VulkanLightManager::Initialize(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext)
	{
		ME_PROFILE_FUNCTION()

		CreateShadowMapSampler(descriptorContext);
		CreateSkyboxSampler(descriptorContext);

		CreateDefaultShadowMap(cmdPoolManager, descriptorContext, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
		InitLightBuffers();

		LoadSkyBox(cmdPoolManager, descriptorContext);
	}

	void VulkanLightManager::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		for (auto& s : m_ShadowMaps)
		{
			s.Destroy();
		}

		for (auto& l : m_LightBuffers)
		{
			l.buffer.Destroy();
		}

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_ShadowMapSampler, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_SkyboxSampler, nullptr);

		m_Skybox.Destroy();
	}

	uint32_t VulkanLightManager::CreateLight()
	{
		return m_NextLightID++;
	}

	void VulkanLightManager::Draw(VkCommandBuffer const& commandBuffer, VulkanGraphicsPipelineContext const& graphicsPipelineContext, VulkanDescriptorContext& descriptorContext, VulkanSwapchainContext& swapChainContext, uint32_t frame)
	{
		VkClearValue constexpr depthClear{ .depthStencil = { 1.0f, 0 } };

		for (uint32_t lightId{ 0 }; lightId < std::size(m_Lights); ++lightId)
		{
			auto const& light{ m_Lights[lightId] };
			if (light.castsShadows and light.type == static_cast<uint32_t>(MauEng::ELightType::DIRECTIONAL))
			{
				auto& depth = m_ShadowMaps[light.shadowMapIndex];

				if (VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL != depth.layout)
				{
					depth.TransitionImageLayout(
						commandBuffer,
						VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
						VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
						0,
						VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
					);
				}

				ShadowPassPushConstant pc{ .lightIndex = lightId };

				VkRenderingAttachmentInfo depthAttachment{};
				depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				depthAttachment.imageView = depth.imageViews[0];
				depthAttachment.imageLayout = depth.layout;
				depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				depthAttachment.clearValue = depthClear;

				VkRenderingInfo renderInfoDepthPrepass{};
				renderInfoDepthPrepass.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
				renderInfoDepthPrepass.renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE };
				renderInfoDepthPrepass.layerCount = 1;
				renderInfoDepthPrepass.colorAttachmentCount = 0;
				renderInfoDepthPrepass.pColorAttachments = nullptr;
				renderInfoDepthPrepass.pDepthAttachment = &depthAttachment;
				renderInfoDepthPrepass.pStencilAttachment = nullptr;

				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)SHADOW_MAP_SIZE;
				viewport.height = (float)SHADOW_MAP_SIZE;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = { SHADOW_MAP_SIZE, SHADOW_MAP_SIZE };
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

				vkCmdBeginRendering(commandBuffer, &renderInfoDepthPrepass);
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineContext.GetShadowPassPipeline());
					vkCmdPushConstants(
						commandBuffer,
						graphicsPipelineContext.GetShadowPassPipelineLayout(),
						VK_SHADER_STAGE_VERTEX_BIT,
						0,
						sizeof(ShadowPassPushConstant),
						&pc
					);

					VulkanMeshManager::GetInstance().Draw(commandBuffer, graphicsPipelineContext.GetShadowPassPipelineLayout(), 1, &descriptorContext.GetDescriptorSets()[frame], frame);
				vkCmdEndRendering(commandBuffer);

				depth.TransitionImageLayout(
					commandBuffer,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT
				);
			}
		}
	}

	void VulkanLightManager::SetSceneAABBOverride(glm::vec3 const& min, glm::vec3 const& max)
	{
		m_HasAABBBOverride = true;
		m_SceneAABBMin = min;
		m_SceneAABBMax = max;
	}

	void VulkanLightManager::PreQueue(glm::mat4 const& viewProj)
	{
		if (not m_HasAABBBOverride)
		{
			glm::mat4 const invViewProj{ glm::inverse(viewProj) };

			glm::vec3 constexpr ndcCorners[8]
			{
				{-1, -1, 0}, {1, -1, 0},
				{-1,  1, 0}, {1,  1, 0},
				{-1, -1, 1}, {1, -1, 1},
				{-1,  1, 1}, {1,  1, 1}
			};

			glm::vec3 worldCorners[8];
			for (size_t i{ 0 }; i < 8; ++i)
			{
				glm::vec4 worldPos{ invViewProj * glm::vec4(ndcCorners[i], 1.0f) };
				worldCorners[i] = glm::vec3(worldPos) / worldPos.w;
			}

			glm::vec3 sceneAABBMin{ FLT_MAX };
			glm::vec3 sceneAABBMax{ -FLT_MAX };
			for (size_t i{ 0 }; i < 8; ++i)
			{
				sceneAABBMin = glm::min(sceneAABBMin, worldCorners[i]);
				sceneAABBMax = glm::max(sceneAABBMax, worldCorners[i]);
			}
			m_SceneAABBMin = sceneAABBMin;
			m_SceneAABBMax = sceneAABBMax;
		}
	}

	void VulkanLightManager::PreDraw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame)
	{
		{
			ME_PROFILE_SCOPE("Light data update - buffer")
			memcpy(m_LightBuffers[frame].mapped, m_Lights.data(), m_Lights.size() * sizeof(Light));
		}

		{
			ME_PROFILE_SCOPE("Light instance data update - descriptor sets")

			// Will likely never be empty but if it is, skip to prevent errors
			if (not m_Lights.empty())
			{
				auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = m_LightBuffers[frame].buffer.buffer;
				bufferInfo.offset = 0;
				bufferInfo.range = m_Lights.size() * sizeof(Light);

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = *pDescriptorSets;
				descriptorWrite.dstBinding = 12; // Binding index -TODO use a get Binding on the context
				descriptorWrite.dstArrayElement = 0; // Array element offset (if applicable)
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}
	}

	void VulkanLightManager::QueueLight(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, MauEng::CLight const& light)
	{
		if (not light.isEnabled)
		{
			return;
		}

		uint32_t shadowID{ INVALID_SHADOW_MAP_ID };

		if (light.castShadows)
		{
			auto const it{ m_LightShadowMapIDMap.find(light.lightID) };
			if (it != end(m_LightShadowMapIDMap))
			{
				shadowID = it->second;
			}
			else
			{
				shadowID = m_NextShadowMapID;
				CreateShadowMap(cmdPoolManager, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
				descriptorContext.BindShadowMap(shadowID, m_ShadowMaps[shadowID].imageViews[0], m_ShadowMaps[shadowID].layout);

				m_LightShadowMapIDMap.emplace(light.lightID, shadowID);
				m_NextShadowMapID++;
			}
		}

		Light vulkanLight;
		vulkanLight.type = static_cast<uint32_t>(light.type);
		vulkanLight.direction_position = light.direction_position;
		vulkanLight.color = light.lightColour;
		vulkanLight.intensity = light.intensity;
		vulkanLight.shadowMapIndex = shadowID;
		vulkanLight.castsShadows = light.castShadows ? 1 : 0;
		if (0 == vulkanLight.type)
		{
			glm::vec3 const sceneCenter{ (m_SceneAABBMin + m_SceneAABBMax) * .5f };
			glm::vec3 const lightDir{ glm::normalize(vulkanLight.direction_position) };
			// Calc safe up vec (aligned dir and up)
			glm::vec3 const up{
				glm::abs(glm::dot(lightDir, glm::vec3{0.f, 1.f, 0.f})) > .99f
				? glm::vec3{ 0.f, 0.f, 1.f }
				: glm::vec3{ 0.f, 1.f, 0.f }
			};

			std::vector<glm::vec3> const sceneCorners
			{
				{ m_SceneAABBMin.x, m_SceneAABBMin.y, m_SceneAABBMin.z },
				{ m_SceneAABBMax.x, m_SceneAABBMin.y, m_SceneAABBMin.z },
				{ m_SceneAABBMin.x, m_SceneAABBMax.y, m_SceneAABBMin.z },
				{ m_SceneAABBMax.x, m_SceneAABBMax.y, m_SceneAABBMin.z },
				{ m_SceneAABBMin.x, m_SceneAABBMin.y, m_SceneAABBMax.z },
				{ m_SceneAABBMax.x, m_SceneAABBMin.y, m_SceneAABBMax.z },
				{ m_SceneAABBMin.x, m_SceneAABBMax.y, m_SceneAABBMax.z },
				{ m_SceneAABBMax.x, m_SceneAABBMax.y, m_SceneAABBMax.z }
			};

			// Project corners on light dir
			float minProj{ FLT_MAX };
			float maxProj{ -FLT_MAX };
			for (auto const& c : sceneCorners)
			{
				float const proj{ glm::dot(c, lightDir)};

				minProj = std::min(minProj, proj);
				maxProj = std::max(maxProj, proj);
			}

			// Distance & position (even though theres not really a position for dir light)
			float const distance{ maxProj - glm::dot(sceneCenter, lightDir) };
			//float const sceneExtent{ (maxProj - minProj) * 0.5f };
			glm::vec3 const lightPos{ sceneCenter - lightDir * distance };

			// Use lightpos wth centter to gen view mat - inverted Y axis for up
			auto const lightView{ glm::lookAt(lightPos, sceneCenter, up) };

			// Now go over corners again, find min and max in view/light space
			glm::vec3 minLightSpace{ FLT_MAX };
			glm::vec3 maxLightSpace{ -FLT_MAX };
			for (auto const& c : sceneCorners)
			{
				glm::vec3 const tCorner{ glm::vec3{ lightView * glm::vec4{ c, 1.f} } };

				minLightSpace = glm::min(minLightSpace, tCorner);
				maxLightSpace = glm::max(maxLightSpace, tCorner);
			}

			// With min and max, create orthographics proj matrix
			float const nearZ{ 0.f };
			float const farZ{ maxLightSpace.z - minLightSpace.z};

			auto lightProj{
				glm::orthoRH_ZO(minLightSpace.x, maxLightSpace.x,
				minLightSpace.y, maxLightSpace.y, 
				nearZ, farZ)
			};
			
			vulkanLight.lightViewProj = lightProj * lightView;

			if constexpr (DEBUG_RENDER_SCENE_AABB)
			{
				for (auto& c : sceneCorners)
				{
					DEBUG_RENDERER.DrawSphere(c, 10.f);
				}

				DEBUG_RENDERER.DrawSphere(m_SceneAABBMin, 20.f, {}, { 1, 1, 1 });
				DEBUG_RENDERER.DrawSphere(m_SceneAABBMax, 20.f, {}, { 1, 1, 1 });
				DEBUG_RENDERER.DrawSphere(sceneCenter, 50.f, {}, { 1, 1, 0 });
				DEBUG_RENDERER.DrawSphere(lightPos, 40.f, {}, light.lightColour);
			}
		}

		m_Lights.emplace_back(vulkanLight);
	}

	void VulkanLightManager::PostDraw()
	{
		m_Lights.clear();
	}

	void VulkanLightManager::RenderSkybox(VkCommandBuffer const& commandBuffer, VulkanGraphicsPipelineContext const& graphicsPipelineContext, VulkanDescriptorContext& descriptorContext, VulkanSwapchainContext& swapChainContext, uint32_t frame, VulkanBuffer const& screenBuffer)
	{
		if (execOnce)
		{
			return;
		}
		execOnce = true;

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		// CPU views per face
		glm::vec3 const eye{ 0.f };
		glm::mat4 captureViews[6]
		{
			glm::lookAt(eye, eye + glm::vec3{ 1.f, 0.f, 0.f }, glm::vec3{ 0.f, -1.f, 0.f }), // +X
			glm::lookAt(eye, eye + glm::vec3{ -1.f, 0.f, 0.f }, glm::vec3{ 0.f, -1.f, 0.f }), // -X
			glm::lookAt(eye, eye + glm::vec3{ 0.f, -1.f, 0.f }, glm::vec3{ 0.f, 0.f, -1.f }), // -Y
			glm::lookAt(eye, eye + glm::vec3{ 0.f, 1.f, 0.f }, glm::vec3{ 0.f, 0.f, 1.f }), // +Y
			glm::lookAt(eye, eye + glm::vec3{ 0.f, 0.f, 1.f }, glm::vec3{ 0.f, -1.f, 0.f }), // +Z
			glm::lookAt(eye, eye + glm::vec3{ 0.f, 0.f, -1.f }, glm::vec3{ 0.f, -1.f, 0.f })  // -Z
		};
		glm::mat4 captureProj{ glm::perspective(glm::radians(90.f), 1.f, .1f, 10.f) };
		captureProj[1][1] *= -1.f;

		VkImageView cubemapViews[6];
		for (size_t i{ 0 }; i < 6; ++i)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_Skybox.image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = m_Skybox.format;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = i;
			viewInfo.subresourceRange.layerCount = 1;

			vkCreateImageView(deviceContext->GetLogicalDevice(), &viewInfo, nullptr, &cubemapViews[i]);
		}

		m_Skybox.TransitionImageLayout(commandBuffer, 
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_2_SHADER_READ_BIT,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
			);

		for (size_t i{ 0 }; i < 6; ++i)
		{
			SkyBoxPushConstant pc
			{
				.view = captureViews[i],
				.proj = captureProj
			};

			auto& depth{ swapChainContext.GetDepthImage(frame) };
			VkRenderingAttachmentInfo colorAttachment{};
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachment.imageView = cubemapViews[i];
			colorAttachment.imageLayout = m_Skybox.layout;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.clearValue = { 1,1,1,1 };

			VkRenderingAttachmentInfo depthAttachment{};
			depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachment.imageView = depth.imageViews[0];
			depthAttachment.imageLayout = depth.layout;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			VkRenderingInfo renderInfoSkybox{};
			renderInfoSkybox.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderInfoSkybox.renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, swapChainContext.GetExtent().width, swapChainContext.GetExtent().height };
			renderInfoSkybox.layerCount = 1;
			renderInfoSkybox.colorAttachmentCount = 1;
			renderInfoSkybox.pColorAttachments = &colorAttachment;
			renderInfoSkybox.pDepthAttachment = &depthAttachment;

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)swapChainContext.GetExtent().width;
			viewport.height = (float)swapChainContext.GetExtent().height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = { swapChainContext.GetExtent().width, swapChainContext.GetExtent().height };
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
			VkDeviceSize constexpr offset{ 0 };

			vkCmdBeginRendering(commandBuffer, &renderInfoSkybox);
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineContext.GetSkyboxPipeline());
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineContext.GetSkyboxPipelineLayout(), 0, 1, descriptorContext.GetSkyboxDescriptorSets().data(), 0, nullptr);
				vkCmdPushConstants(
					commandBuffer,
					graphicsPipelineContext.GetSkyboxPipelineLayout(),
					VK_SHADER_STAGE_VERTEX_BIT,
					0,
					sizeof(SkyBoxPushConstant),
					&pc
				);
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &screenBuffer.buffer, &offset);
				vkCmdDraw(commandBuffer, 6, 1, 0, 0);
			vkCmdEndRendering(commandBuffer);
		}


		for (size_t i = 0; i < 6; ++i)
		{
			vkDestroyImageView(deviceContext->GetLogicalDevice(), cubemapViews[i], nullptr);
		}

		m_Skybox.TransitionImageLayout(commandBuffer,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_2_SHADER_READ_BIT);

	}

	void VulkanLightManager::CreateShadowMapSampler(VulkanDescriptorContext& descriptorContext)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		// If addressed outside of bounds, repeat (tileable texture)
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(deviceContext->GetPhysicalDevice(), &properties);

		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_TRUE;
		samplerInfo.compareOp = VK_COMPARE_OP_LESS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.minLod = 0.f;
		samplerInfo.maxLod = 1.0f;

		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.mipLodBias = 0.0f;

		if (vkCreateSampler(deviceContext->GetLogicalDevice(), &samplerInfo, nullptr, &m_ShadowMapSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create ShadowMap sampler!");
		}

		descriptorContext.BindShadowMapSampler(m_ShadowMapSampler);
	}

	void VulkanLightManager::CreateDefaultShadowMap(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, uint32_t width, uint32_t height)
	{
		VulkanImage shadowImage
		{
			VK_FORMAT_D32_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			width,
			height,
			1
		};

		shadowImage.CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

		// Transition to transfer dst for clearing
		shadowImage.TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkClearDepthStencilValue clearValue{};
		clearValue.depth = 1.0f;
		clearValue.stencil = 0;

		VkImageSubresourceRange range{};
		range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		auto cmd{ cmdPoolManager.BeginSingleTimeCommands() };
		vkCmdClearDepthStencilImage(
			cmd,
			shadowImage.image,
			shadowImage.layout,
			&clearValue,
			1,
			&range);

		cmdPoolManager.EndSingleTimeCommands(cmd);

		shadowImage.TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_ShadowMaps.emplace_back(shadowImage);

		descriptorContext.BindShadowMap(0, m_ShadowMaps[0].imageViews[0], m_ShadowMaps[0].layout);
	}

	void VulkanLightManager::CreateShadowMap(VulkanCommandPoolManager& cmdPoolManager, uint32_t width, uint32_t height)
	{
		VulkanImage shadowImage
		{
			VK_FORMAT_D32_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			width,
			height,
			1
		};

		shadowImage.CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

		m_ShadowMaps.emplace_back(shadowImage);

		m_ShadowMaps.back().TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanLightManager::InitLightBuffers()
	{
		auto deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize constexpr BUFFER_SIZE{ sizeof(Light) * MAX_LIGHTS };

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_LightBuffers.emplace_back(VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vkMapMemory(deviceContext->GetLogicalDevice(), m_LightBuffers[i].buffer.bufferMemory, 0, BUFFER_SIZE, 0, &m_LightBuffers[i].mapped);
		}
	}

	void VulkanLightManager::LoadSkyBox(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext)
	{
		HDRI_Image img{ "Resources/Skybox/circus_arena_4k.hdr", 4 };

		uint64_t const imageSize{ static_cast<uint64_t>(img.width) * img.height * 4 * sizeof(float) };
		uint32_t const faceSize{ static_cast<uint32_t>(img.height) };

		m_Skybox = VulkanImage{
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			static_cast<uint32_t>(img.height),
			static_cast<uint32_t>(img.height),
			1,
			6,
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
		};
		m_Skybox.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);

		m_Skybox.TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		descriptorContext.BindSkybox(m_Skybox.imageViews[0], m_Skybox.layout);
		descriptorContext.BindEnvMap(m_Skybox.imageViews[0], m_Skybox.layout);
	}

	void VulkanLightManager::CreateSkyboxSampler(VulkanDescriptorContext& descriptorContext)
	{
		auto const deviceContext = VulkanDeviceContextManager::GetInstance().GetDeviceContext();

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_FALSE; // Optional: can be VK_TRUE if supported and desired
		samplerInfo.maxAnisotropy = 1.0f; // Ignored if anisotropyEnable == VK_FALSE
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Not used with clamp to edge
		samplerInfo.unnormalizedCoordinates = VK_FALSE; // Use normalized texture coordinates
		samplerInfo.compareEnable = VK_FALSE; // No compare operation needed
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // Ignored since compareEnable is VK_FALSE
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Use linear mipmapping
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = VK_LOD_CLAMP_NONE; // Use all mip levels available

		if (vkCreateSampler(deviceContext->GetLogicalDevice(), &samplerInfo, nullptr, &m_SkyboxSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create skybox sampler!");
		}

		descriptorContext.BindSkyboxSampler(m_SkyboxSampler);
	}
}
