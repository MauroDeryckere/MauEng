#include "VulkanRenderer.h"

#include "VulkanUtils.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include "VulkanMeshManager.h"
#include "VulkanMaterialManager.h"
#include "DebugRenderer/InternalDebugRenderer.h"
#include "DebugRenderer/NullDebugRenderer.h"

#include "../../MauEng/Public/Components/CStaticMesh.h"

namespace MauRen
{
	VulkanRenderer::VulkanRenderer(SDL_Window* pWindow, DebugRenderer& debugRenderer) :
		Renderer{ pWindow, debugRenderer },
		m_pWindow{ pWindow }
	{
		ME_PROFILE_FUNCTION();

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
		m_GraphicsPipeline.Initialize(&m_SwapChainContext, m_DescriptorContext.GetDescriptorSetLayout(), 1u);

		m_CommandPoolManager.Initialize();

		m_SwapChainContext.InitializeResourcesAndCreateFrames(&m_GraphicsPipeline);

		VulkanMaterialManager::GetInstance().Initialize();

		CreateUniformBuffers();

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

		m_CommandPoolManager.Destroy();

		m_SwapChainContext.Destroy();

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_MappedUniformBuffers[i].buffer.Destroy();
		}


		m_GraphicsPipeline.Destroy();

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
		VulkanMeshManager::GetInstance().QueueDraw(transformMat, mesh.meshID, mesh.materialID);
	}

	MeshInstance VulkanRenderer::LoadOrGetMeshData(char const* path)
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
			
		VkCommandBufferBeginInfo beginInfo{};

		// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
		// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
		// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : The command buffer can be resubmitted while it is also already pending execution.
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional; only relevant for secondary

		//Note: if the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset it.
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_GraphicsPipeline.GetRenderPass();
		renderPassInfo.framebuffer = m_SwapChainContext.GetSwapchainFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChainContext.GetExtent();

		VkClearColorValue constexpr clearColor{ 0.0f, 0.0f, 0.0f, 1.f };
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = clearColor;
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.GetPipeline() );

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(m_SwapChainContext.GetExtent().width);
			viewport.height = static_cast<float>(m_SwapChainContext.GetExtent().height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = m_SwapChainContext.GetExtent();
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VulkanMeshManager::GetInstance().Draw(commandBuffer, m_GraphicsPipeline.GetPipelineLayout(), 1, &m_DescriptorContext.GetDescriptorSets()[m_CurrentFrame], m_CurrentFrame);
		RenderDebug(commandBuffer);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to record command buffer!");
		}

		//TODO
		/*Driver developers recommend that you also store multiple buffers, like the vertex and index buffer, into a single VkBuffer
		 *and use offsets in commands like vkCmdBindVertexBuffers.
		 *The advantage is that your data is more cache friendly in that case, because it's closer together.
		 *It is even possible to reuse the same chunk of memory for multiple resources if they are not used during the same render operations,
		 *provided that their data is refreshed, of course. This is known as aliasing and some Vulkan functions have explicit flags to specify that you want to do this.
		 */
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
			if (vkCreateSemaphore(deviceContext->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS
				|| vkCreateSemaphore(deviceContext->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS
				|| vkCreateFence(deviceContext->GetLogicalDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
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

			if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
			{
				RecreateSwapchain();
				return;
			}

			// TODO
			// You could also decide to do that if the swap chain is suboptimal, but I've chosen to proceed anyway in that case because we've already acquired an image.
			// Both VK_SUCCESS and VK_SUBOPTIMAL_KHR are considered "success" return codes.
			if (acquireNextImageResult != VK_SUCCESS && acquireNextImageResult != VK_SUBOPTIMAL_KHR)
			{
				throw std::runtime_error("Failed to acquire swap chain image!");
			}

			// Only reset the fence if we are submitting work
			vkResetFences(deviceContext->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame]);
		}

		UpdateUniformBuffer(m_CurrentFrame, view, proj);
		UpdateDebugVertexBuffer();

		vkResetCommandBuffer(m_CommandPoolManager.GetCommandBuffer(m_CurrentFrame), 0);
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

		// m_InFlightFences here effectively means, this submit must be finished before our next render may start
		if (vkQueueSubmit(deviceContext->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
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
		if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
		{
			RecreateSwapchain();
		}
		
		// TODO
		// You could also decide to do that if the swap chain is suboptimal, but I've chosen to proceed anyway in that case because we've already acquired an image.
		// Both VK_SUCCESS and VK_SUBOPTIMAL_KHR are considered "success" return codes.
		else if (queuePresentResult != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to present swap chain image!");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage, glm::mat4 const& view, glm::mat4 const& proj)
	{
		ME_PROFILE_FUNCTION()
		UniformBufferObject const ubo{ view, proj };
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
				if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
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
		m_SwapChainContext.ReCreate(m_pWindow, &m_GraphicsPipeline, &m_SurfaceContext);

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

	void VulkanRenderer::RenderDebug(VkCommandBuffer commandBuffer)
	{
		ME_PROFILE_FUNCTION()

		if (!m_DebugRenderer)
		{
			return;
		}

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.GetDebugPipeline());

		VkDeviceSize constexpr offset{ 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_DebugVertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(commandBuffer, m_DebugIndexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_DebugRenderer->m_IndexBuffer.size()), 1, 0, 0, 0);

	}
}


