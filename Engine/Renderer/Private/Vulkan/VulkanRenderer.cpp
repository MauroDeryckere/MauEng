#include "VulkanRenderer.h"

#include "Assets/VulkanMeshManager.h"
#include "Assets/VulkanMaterialManager.h"
#include "DebugRenderer/InternalDebugRenderer.h"
#include "DebugRenderer/NullDebugRenderer.h"

#include "../../MauEng/Public/Components/CStaticMesh.h"

namespace MauRen
{
	VulkanRenderer::VulkanRenderer(SDL_Window* pWindow, DebugRenderer& debugRenderer) :
		Renderer{ pWindow, debugRenderer },
		m_pWindow{ pWindow }
	{
		ME_PROFILE_FUNCTION()

		if (dynamic_cast<NullDebugRenderer*>(&debugRenderer))
		{
			m_DebugRenderer = nullptr;
		}
		else
		{
			m_DebugRenderer = &static_cast<InternalDebugRenderer&>(debugRenderer);
			ME_RENDERER_ASSERT(m_DebugRenderer);
		}
	}

	void VulkanRenderer::Init()
	{
		ME_PROFILE_FUNCTION()

		m_InstanceContext.Initialize();
		m_SurfaceContext.Initialize(&m_InstanceContext, m_pWindow);
		m_DebugContext.Initialize(&m_InstanceContext);

		VulkanDeviceContextManager::GetInstance().Initialize(&m_SurfaceContext, &m_InstanceContext);

		m_DescriptorContext.Initialize();
		m_SwapChainContext.Initialize(m_pWindow, &m_SurfaceContext);

		m_DescriptorContext.CreateDescriptorSetLayout();
		m_GraphicsPipelineContext.Initialize(&m_SwapChainContext, m_DescriptorContext.GetDescriptorSetLayout(), 1u);

		m_CommandPoolManager.Initialize();


		CreateUniformBuffers();
		VulkanMaterialManager::GetInstance().Initialize();

		m_DescriptorContext.CreateDescriptorPool();
		std::vector<VulkanBuffer> tempUniformBuffers;
		for (auto const& b : m_MappedUniformBuffers)
		{
			tempUniformBuffers.emplace_back(b.buffer);
		}

		m_DescriptorContext.CreateDescriptorSets(tempUniformBuffers,
			0,
			sizeof(UniformBufferObject),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, {}, VulkanMaterialManager::GetInstance().GetTextureSampler());

		m_CommandPoolManager.CreateCommandBuffers();
		CreateSyncObjects();

		VulkanMaterialManager::GetInstance().InitializeTextureManager(m_CommandPoolManager, m_DescriptorContext);
		VulkanMeshManager::GetInstance().Initialize(&m_CommandPoolManager);

		if (m_DebugRenderer)
		{
			size_t bufferSize = sizeof(m_DebugRenderer->m_ActivePoints[0]) * m_DebugRenderer->MAX_LINES;

			m_DebugVertexBuffer = { bufferSize,
									  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
									  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

			m_DebugIndexBuffer = { bufferSize,
									  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
									  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
		}

		m_QuadVertexBuffer = { sizeof(m_QuadVertices[0]) * std::size(m_QuadVertices),
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		};

		VulkanBuffer stagingBuffer
		{
			sizeof(m_QuadVertices[0]) * std::size(m_QuadVertices),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		void* mappedMemory;
		vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, sizeof(m_QuadVertices[0]) * std::size(m_QuadVertices), 0, &mappedMemory);

		// Copy the data to the buffer
		memcpy(mappedMemory, m_QuadVertices.data(), sizeof(m_QuadVertices[0]) * std::size(m_QuadVertices));

		// Unmap the memory
		vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);

		VulkanBuffer::CopyBuffer(m_CommandPoolManager, stagingBuffer.buffer, m_QuadVertexBuffer.buffer, sizeof(m_QuadVertices[0]) * std::size(m_QuadVertices));
		stagingBuffer.Destroy();
	}

	void VulkanRenderer::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		// Wait for GPU to finish everything
		vkDeviceWaitIdle(deviceContext->GetLogicalDevice());

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(deviceContext->GetLogicalDevice(), m_ImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(deviceContext->GetLogicalDevice(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(deviceContext->GetLogicalDevice(), m_InFlightFences[i], nullptr);
		}

		VulkanMaterialManager::GetInstance().Destroy();
		VulkanMeshManager::GetInstance().Destroy();

		if (m_DebugRenderer)
		{
			m_DebugVertexBuffer.Destroy();
			m_DebugIndexBuffer.Destroy();
		}

		m_QuadVertexBuffer.Destroy();

		m_CommandPoolManager.Destroy();

		m_SwapChainContext.Destroy();

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_MappedUniformBuffers[i].buffer.Destroy();
		}

		m_GraphicsPipelineContext.Destroy();

		m_SwapChainContext.Destroy();
		m_DescriptorContext.Destroy();

		VulkanDeviceContextManager::GetInstance().Destroy();

		m_DebugContext.Destroy();
		m_SurfaceContext.Destroy();
		m_InstanceContext.Destroy();
	}

	void VulkanRenderer::Render(glm::mat4 const& view, glm::mat4 const& proj)
	{
		DrawFrame(view, proj);

		if (m_DebugRenderer)
		{
			m_DebugRenderer->m_ActivePoints.clear();
			m_DebugRenderer->m_IndexBuffer.clear();
		}
	}

	void VulkanRenderer::ResizeWindow()
	{
		m_FramebufferResized = true;
	}

	void VulkanRenderer::QueueDraw(glm::mat4 const& transformMat, MauEng::CStaticMesh const& mesh)
	{
		VulkanMeshManager::GetInstance().QueueDraw(transformMat, mesh.meshID);
	}

	uint32_t VulkanRenderer::LoadOrGetMeshID(char const* path)
	{
		return VulkanMeshManager::GetInstance().LoadMesh(path, m_CommandPoolManager, m_DescriptorContext);
	}

	void VulkanRenderer::CreateUniformBuffers()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize constexpr BUFFER_SIZE{ sizeof(UniformBufferObject) };

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_MappedUniformBuffers.emplace_back(VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vkMapMemory(deviceContext->GetLogicalDevice(), m_MappedUniformBuffers[i].buffer.bufferMemory, 0, BUFFER_SIZE, 0, &m_MappedUniformBuffers[i].mapped);
		}
	}

	void VulkanRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		ME_PROFILE_FUNCTION()
#pragma region PRE_DRAW
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional; only relevant for secondary

		//Note: if the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset it.
		if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &beginInfo))
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		
		// Image memory barriers
		auto& depth{ m_SwapChainContext.GetDepthImage(imageIndex) };
		auto& colour{ m_SwapChainContext.GetColorImage(imageIndex) };

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_SwapChainContext.GetExtent().width);
		viewport.height = static_cast<float>(m_SwapChainContext.GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_SwapChainContext.GetExtent();

		VulkanMeshManager::GetInstance().PreDraw(commandBuffer, m_GraphicsPipelineContext.GetForwardPipelineLayout(), 1, &m_DescriptorContext.GetDescriptorSets()[m_CurrentFrame], m_CurrentFrame);

		auto& gBufferColor{ m_SwapChainContext.GetGBuffer(m_CurrentFrame).color };
		// GBuffer Colour
		if (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL != gBufferColor.layout)
		{
			gBufferColor.TransitionImageLayout(commandBuffer,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
		}

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageView = gBufferColor.imageViews[0];
		imageInfo.imageLayout = gBufferColor.layout;

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_DescriptorContext.GetDescriptorSets()[m_CurrentFrame];
		descriptorWrite.dstBinding = 6;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);

#pragma endregion
#pragma region DEPTH_PREPASS
		{
			ME_PROFILE_SCOPE("Depth Prepass")
			// Depth
			depth.TransitionImageLayout(commandBuffer,
				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
				VK_ACCESS_2_NONE, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

			VkRenderingAttachmentInfo depthAttachment{};
			depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachment.imageView = depth.imageViews[0];
			depthAttachment.imageLayout = depth.layout;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.clearValue = CLEAR_VALUES[DEPTH_CLEAR_ID];

			VkRenderingInfo renderInfoDepthPrepass{};
			renderInfoDepthPrepass.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderInfoDepthPrepass.renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_SwapChainContext.GetExtent() };
			renderInfoDepthPrepass.layerCount = 1;
			renderInfoDepthPrepass.colorAttachmentCount = 0;
			renderInfoDepthPrepass.pColorAttachments = nullptr;
			renderInfoDepthPrepass.pDepthAttachment = &depthAttachment;
			renderInfoDepthPrepass.pStencilAttachment = nullptr;
			
			vkCmdBeginRendering(commandBuffer, &renderInfoDepthPrepass);
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipelineContext.GetDepthPrePassPipeline());
				VulkanMeshManager::GetInstance().Draw(commandBuffer, m_GraphicsPipelineContext.GetDepthPrePassPipelineLayout(), 1, &m_DescriptorContext.GetDescriptorSets()[m_CurrentFrame], m_CurrentFrame);
				RenderDebug(commandBuffer, true);
			vkCmdEndRendering(commandBuffer);
		}
#pragma endregion

#pragma region GBUFFER_PASS
		{
			ME_PROFILE_SCOPE("GBuffer pass")

			// GBuffer Colour
			if (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL != gBufferColor.layout)
			{
				gBufferColor.TransitionImageLayout(commandBuffer,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
			}

			// Depth
			if (VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL != depth.layout)
			{
				depth.TransitionImageLayout(commandBuffer,
					VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
					VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
					VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT);
			}

			VkRenderingAttachmentInfo colorAttachment = {};
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachment.imageView = gBufferColor.imageViews[0];
			colorAttachment.imageLayout = gBufferColor.layout;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.clearValue = CLEAR_VALUES[COLOR_CLEAR_ID];

			VkRenderingAttachmentInfo depthAttachment{};
			depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachment.imageView = depth.imageViews[0];
			depthAttachment.imageLayout = depth.layout;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.clearValue = CLEAR_VALUES[DEPTH_CLEAR_ID];

			VkRenderingInfo renderInfoGBuffer{};
			renderInfoGBuffer.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderInfoGBuffer.renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_SwapChainContext.GetExtent() };
			renderInfoGBuffer.layerCount = 1;
			renderInfoGBuffer.colorAttachmentCount = 1;
			renderInfoGBuffer.pColorAttachments = &colorAttachment;
			renderInfoGBuffer.pDepthAttachment = &depthAttachment;
			renderInfoGBuffer.pStencilAttachment = nullptr;

			vkCmdBeginRendering(commandBuffer, &renderInfoGBuffer);
				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipelineContext.GetGBufferPipeline());
				VulkanMeshManager::GetInstance().Draw(commandBuffer, m_GraphicsPipelineContext.GetGBufferPipelineLayout(), 1, &m_DescriptorContext.GetDescriptorSets()[m_CurrentFrame], m_CurrentFrame);
			vkCmdEndRendering(commandBuffer);
		}
#pragma endregion
		//TODO
		//RenderDebug(commandBuffer, false);

#pragma region LIGHTING_PASS
		{
			ME_PROFILE_SCOPE("lighting pass")
			// Colour
			if (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL != colour.layout)
			{
				colour.TransitionImageLayout(commandBuffer,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
			}

			// GBuffer Colour
			if (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL != gBufferColor.layout)
			{
				gBufferColor.TransitionImageLayout(commandBuffer,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
					VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT);
			}

			VkRenderingAttachmentInfo colorAttachment{};
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachment.imageView = colour.imageViews[0];
			colorAttachment.imageLayout = colour.layout;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.clearValue = CLEAR_VALUES[COLOR_CLEAR_ID];

			//VkRenderingAttachmentInfo depthAttachment{};
			//depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			//depthAttachment.imageView = depth.imageViews[0];
			//depthAttachment.imageLayout = depth.layout;
			//depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			//depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			VkRenderingInfo renderInfo{};
			renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderInfo.renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_SwapChainContext.GetExtent() };
			renderInfo.layerCount = 1;
			renderInfo.colorAttachmentCount = 1;
			renderInfo.pColorAttachments = &colorAttachment;
			renderInfo.pDepthAttachment = nullptr;
			renderInfo.pStencilAttachment = nullptr;

			vkCmdBeginRendering(commandBuffer, &renderInfo);
				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
				VkDeviceSize constexpr offset{ 0 };

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipelineContext.GetLightingPipeline());
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipelineContext.GetLightingPipelineLayout(), 0, 1, &m_DescriptorContext.GetDescriptorSets()[m_CurrentFrame], 0, nullptr);
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_QuadVertexBuffer.buffer, &offset);
				vkCmdDraw(commandBuffer, 6, 1, 0, 0);
				//VulkanMeshManager::GetInstance().Draw(commandBuffer, m_GraphicsPipelineContext.GetLightingPipelineLayout(), 1, &m_DescriptorContext.GetDescriptorSets()[m_CurrentFrame], m_CurrentFrame);
			vkCmdEndRendering(commandBuffer);
		}
#pragma endregion
#pragma region POST_DRAW
		{
			ME_PROFILE_SCOPE("Post draw")

			VulkanMeshManager::GetInstance().PostDraw(commandBuffer, m_GraphicsPipelineContext.GetForwardPipelineLayout(), 1, &m_DescriptorContext.GetDescriptorSets()[m_CurrentFrame], m_CurrentFrame);

			//// Transition color layout to transfer optimal before resolving
			//if (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL != colour.layout)
			//{
			//	colour.TransitionImageLayout(commandBuffer,
			//		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			//		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			//		VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			//		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			//		VK_ACCESS_2_TRANSFER_READ_BIT);
			//}

			//VkImageResolve resolveRegion = {};
			//resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			//resolveRegion.srcSubresource.mipLevel = 0;
			//resolveRegion.srcSubresource.baseArrayLayer = 0;
			//resolveRegion.srcSubresource.layerCount = 1;
			//resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			//resolveRegion.dstSubresource.mipLevel = 0;
			//resolveRegion.dstSubresource.baseArrayLayer = 0;
			//resolveRegion.dstSubresource.layerCount = 1;
			//resolveRegion.extent = { m_SwapChainContext.GetExtent().width, m_SwapChainContext.GetExtent().height, 1 };

			//vkCmdResolveImage(commandBuffer,
			//	colour.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			//	m_SwapChainContext.GetSwapchainImages()[imageIndex].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			//	1, &resolveRegion);

			colour.TransitionImageLayout(
				commandBuffer,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_2_TRANSFER_READ_BIT
			);

			if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL != m_SwapChainContext.GetSwapchainImages()[imageIndex].layout)
			{
				m_SwapChainContext.GetSwapchainImages()[imageIndex].TransitionImageLayout(commandBuffer,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
					VK_ACCESS_2_NONE, VK_ACCESS_2_TRANSFER_WRITE_BIT);
			}

			VkImageCopy copyRegion = {};
			copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = { 0, 0, 0 };

			copyRegion.dstSubresource = copyRegion.srcSubresource;
			copyRegion.dstOffset = { 0, 0, 0 };

			copyRegion.extent = { m_SwapChainContext.GetExtent().width, m_SwapChainContext.GetExtent().height, 1 }; // Match your output resolution

			vkCmdCopyImage(
				commandBuffer,
				colour.image, colour.layout,
				m_SwapChainContext.GetSwapchainImages()[imageIndex].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &copyRegion
			);
			m_SwapChainContext.GetSwapchainImages()[imageIndex].layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			m_SwapChainContext.GetSwapchainImages()[imageIndex].TransitionImageLayout(commandBuffer,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
				VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_ACCESS_2_MEMORY_READ_BIT);

			if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer))
			{
				throw std::runtime_error("Failed to record command buffer!");
			}
		}
#pragma endregion
	}

	void VulkanRenderer::CreateSyncObjects()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// Create the fence in the signaled state, so that the first call to vkWaitForFences() returns immediately since the fence is already signaled
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			if (VK_SUCCESS != vkCreateSemaphore(deviceContext->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i])
			 or VK_SUCCESS != vkCreateSemaphore(deviceContext->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i])
			 or VK_SUCCESS != vkCreateFence(deviceContext->GetLogicalDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]))
			{
				throw std::runtime_error("Failed to create synchronization objects for a frame!");
			}
		}
	}

	void VulkanRenderer::DrawFrame(glm::mat4 const& view, glm::mat4 const& proj)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		{
			ME_PROFILE_SCOPE("Wait for GPU")
			// At the start of the frame, we want to wait until the previous frame has finished, so that the command buffer and semaphores are available to use.
			vkWaitForFences(deviceContext->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
		}

		uint32_t imageIndex;
		{
			ME_PROFILE_SCOPE("acquireNextImageResult")

			VkResult const acquireNextImageResult{ vkAcquireNextImageKHR(deviceContext->GetLogicalDevice(), m_SwapChainContext.GetSwapchain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex) };

			if (VK_ERROR_OUT_OF_DATE_KHR == acquireNextImageResult)
			{
				RecreateSwapchain();
				return;
			}

			// TODO
			// You could also decide to do that if the swap chain is suboptimal, but I've chosen to proceed anyway in that case because we've already acquired an image.
			// Both VK_SUCCESS and VK_SUBOPTIMAL_KHR are considered "success" return codes.
			if (VK_SUCCESS != acquireNextImageResult 
			and VK_SUBOPTIMAL_KHR != acquireNextImageResult)
			{
				throw std::runtime_error("Failed to acquire swap chain image!");
			}

			// Only reset the fence if we are submitting work
			vkResetFences(deviceContext->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame]);
		}

		UpdateUniformBuffer(m_CurrentFrame, view, proj);
		UpdateDebugVertexBuffer();
		{
			ME_PROFILE_SCOPE("Reset command buffer")
			vkResetCommandBuffer(m_CommandPoolManager.GetCommandBuffer(m_CurrentFrame), 0);
		}

		RecordCommandBuffer(m_CommandPoolManager.GetCommandBuffer(m_CurrentFrame), imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore const waitSemaphores[] { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags constexpr waitStages[] { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandPoolManager.GetCommandBuffer(m_CurrentFrame);

		VkSemaphore const signalSemaphores[] { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		//vkQueueSubmit2();
		// m_InFlightFences here effectively means, this submit must be finished before our next render may start
		if (VK_SUCCESS != vkQueueSubmit(deviceContext->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]))
		{
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		// The first two parameters specify which semaphores to wait on before presentation can happen
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR const swapChains[] { m_SwapChainContext.GetSwapchain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		// Not necessary when using a single swapchain
		presentInfo.pResults = nullptr; // Optional

		VkResult const queuePresentResult{ vkQueuePresentKHR(deviceContext->GetPresentQueue(), &presentInfo) };
		if (VK_ERROR_OUT_OF_DATE_KHR == queuePresentResult || VK_SUBOPTIMAL_KHR == queuePresentResult || m_FramebufferResized)
		{
			RecreateSwapchain();
		}
		
		// TODO
		// You could also decide to do that if the swap chain is suboptimal, but I've chosen to proceed anyway in that case because we've already acquired an image.
		// Both VK_SUCCESS and VK_SUBOPTIMAL_KHR are considered "success" return codes.
		else if (VK_SUCCESS != queuePresentResult)
		{
			throw std::runtime_error("Failed to present swap chain image!");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage, glm::mat4 const& view, glm::mat4 const& proj)
	{
		ME_PROFILE_FUNCTION()

		UniformBufferObject const ubo
		{
				.viewProj = proj * view,
				.cameraPosition = glm::vec3{ glm::inverse(view)[3] }
		};

		memcpy(m_MappedUniformBuffers[currentImage].mapped, &ubo, sizeof(ubo));
	}

	bool VulkanRenderer::RecreateSwapchain()
	{
		ME_PROFILE_FUNCTION()

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		// TODO 
		// It is possible to create a new swap chain while drawing commands on an image from the old swap chain are still in - flight.
		// You need to pass the previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR struct and destroy the old swap chain as soon as you've finished using it.

		if (SDL_GetWindowFlags(m_pWindow) & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN))
		{
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (SDL_EVENT_QUIT == event.type || SDL_EVENT_WINDOW_CLOSE_REQUESTED == event.type)
				{
					return false;
				}
			}
		}

		if (SDL_GetWindowFlags(m_pWindow) & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN))
		{
			return false;
		}

		m_FramebufferResized = false;

		vkDeviceWaitIdle(deviceContext->GetLogicalDevice());
		m_SwapChainContext.ReCreate(m_pWindow, &m_GraphicsPipelineContext, &m_SurfaceContext);

		return true;
	}

	void VulkanRenderer::UpdateDebugVertexBuffer()
	{
		//TODO use VulkanMappedBuffer

		ME_PROFILE_FUNCTION()

		if (!m_DebugRenderer)
		{
			return;
		}

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		if (m_DebugRenderer->m_ActivePoints.empty())
		{
			return;
		}

		// Map the vertex buffer memory
		{
			size_t const bufferSize{ sizeof(m_DebugRenderer->m_ActivePoints[0]) * m_DebugRenderer->m_ActivePoints.size() };

			VulkanBuffer stagingBuffer
			{
				bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT // Make it accessible to CPU
			};

			void* mappedMemory;
			vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, bufferSize, 0, &mappedMemory);

			// Copy the data to the buffer
			memcpy(mappedMemory, m_DebugRenderer->m_ActivePoints.data(), bufferSize);

			// Unmap the memory
			vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);

			VulkanBuffer::CopyBuffer(m_CommandPoolManager, stagingBuffer.buffer, m_DebugVertexBuffer.buffer, bufferSize);
			stagingBuffer.Destroy();
		}

		{
			size_t const bufferSize{ sizeof(m_DebugRenderer->m_IndexBuffer[0]) * m_DebugRenderer->m_IndexBuffer.size() };

			VulkanBuffer stagingBuffer
			{
				bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT // Make it accessible to CPU
			};

			void* mappedMemory;
			vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, bufferSize, 0, &mappedMemory);

			// Copy the data to the buffer
			memcpy(mappedMemory, m_DebugRenderer->m_IndexBuffer.data(), bufferSize);

			// Unmap the memory
			vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);

			VulkanBuffer::CopyBuffer(m_CommandPoolManager, stagingBuffer.buffer, m_DebugIndexBuffer.buffer, bufferSize);
			stagingBuffer.Destroy();
		}

	}

	void VulkanRenderer::RenderDebug(VkCommandBuffer commandBuffer, bool isPrepass)
	{
		ME_PROFILE_FUNCTION()

		if (!m_DebugRenderer)
		{
			return;
		}

		if (isPrepass)
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipelineContext.GetDepthPrePassPipeline());
		}
		else
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipelineContext.GetDebugPipeline());
		}

		VkDeviceSize constexpr offset{ 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_DebugVertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(commandBuffer, m_DebugIndexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_DebugRenderer->m_IndexBuffer.size()), 1, 0, 0, 0);

	}
}


