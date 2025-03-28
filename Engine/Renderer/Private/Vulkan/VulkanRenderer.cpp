#include "VulkanRenderer.h"

#include "VulkanUtils.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace MauRen
{
	VulkanRenderer::VulkanRenderer(GLFWwindow* pWindow) :
		Renderer{ pWindow },
		m_pWindow{ pWindow }
	{
		m_InstanceContext.Initialize();
		m_SurfaceContext.Initialize(&m_InstanceContext, pWindow);
		m_DebugContext.Initialize(&m_InstanceContext);

		VulkanDeviceContextManager::GetInstance().Initialize(&m_SurfaceContext, &m_InstanceContext);

		m_DescriptorContext.Initialize();
		m_SwapChainContext.Initialize(pWindow, &m_SurfaceContext);

		m_DescriptorContext.CreateDescriptorSetLayout();
		m_GraphicsPipeline.Initialize( &m_SwapChainContext, m_DescriptorContext.GetDescriptorSetLayout(), 1u);

		m_CommandPoolManager.Initialize();

		CreateColorResources();
		CreateDepthResources();
		CreateFrameBuffers();

		CreateTextureImage();
		CreateTextureSampler();


		Utils::LoadModel("Models/VikingRoom.obj", m_Vertices, m_Indices);

		CreateVertexBuffer();
		CreateIndexBuffer();
		CreateUniformBuffers();

		//CreateGlobalBuffers();

		m_DescriptorContext.CreateDescriptorPool();
		std::vector<VulkanBuffer> tempUniformBuffers;
		for (auto const& b : m_MappedUniformBuffers)
		{
			tempUniformBuffers.emplace_back(b.buffer);
		}
		m_DescriptorContext.CreateDescriptorSets(tempUniformBuffers,
												0, 
												sizeof(UniformBufferObject), 
												VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
												m_TextureImage.imageViews, 
												m_TextureSampler);

		m_CommandPoolManager.CreateCommandBuffers();
		CreateSyncObjects();
	}

	VulkanRenderer::~VulkanRenderer()
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

		m_IndexBuffer.Destroy();
		m_VertexBuffer.Destroy();


		vkDestroySampler(deviceContext->GetLogicalDevice(), m_TextureSampler, nullptr);

		m_TextureImage.Destroy();

		m_CommandPoolManager.Destroy();

		CleanupSwapchain();

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

	void VulkanRenderer::Render()
	{
		DrawFrame();
	}

	void VulkanRenderer::ResizeWindow()
	{
		m_FramebufferResized = true;
	}

	void VulkanRenderer::CreateFrameBuffers()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		auto const& imageViews{ m_SwapChainContext.GetImageViews() };

		m_SwapChainFramebuffers.resize(imageViews.size());

		for (size_t i{ 0 }; i < imageViews.size(); ++i)
		{
			std::array<VkImageView, 3> const attachments{ m_ColorImage.imageViews[0], m_DepthImage.imageViews[0],  imageViews[i]};
			
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_GraphicsPipeline.GetRenderPass();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_SwapChainContext.GetExtent().width;
			framebufferInfo.height = m_SwapChainContext.GetExtent().height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(deviceContext->GetLogicalDevice(), &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}
	}



	void VulkanRenderer::CreateVertexBuffer()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize const bufferSize{ sizeof(m_Vertices[0]) * m_Vertices.size() };

		VulkanBuffer stagingBuffer{ bufferSize,
									VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		if (stagingBuffer.buffer == VK_NULL_HANDLE || stagingBuffer.bufferMemory == VK_NULL_HANDLE) 
		{
			throw std::runtime_error("Failed to create staging buffer.");
		}


		void* data;
		if (vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, bufferSize, 0, &data) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to map Vulkan buffer memory.");
		}

		memcpy(data, m_Vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);
		
		m_VertexBuffer = { bufferSize,
							VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

		CopyBuffer(stagingBuffer.buffer, m_VertexBuffer.buffer, bufferSize);

		stagingBuffer.Destroy();
	}

	void VulkanRenderer::CreateIndexBuffer()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize const bufferSize{ sizeof(m_Indices[0]) * m_Indices.size() };

		VulkanBuffer stagingBuffer{ bufferSize,
									VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		void* data;
		vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, m_Indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);


		m_IndexBuffer = { bufferSize,
							 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

		CopyBuffer(stagingBuffer.buffer, m_IndexBuffer.buffer, bufferSize);

		stagingBuffer.Destroy();
	}

	void VulkanRenderer::CreateUniformBuffers()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize constexpr bufferSize{ sizeof(UniformBufferObject) };

		m_MappedUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_MappedUniformBuffers[i].buffer = { bufferSize,
												VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
												VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

			// Persistent mapping
			vkMapMemory(deviceContext->GetLogicalDevice(), m_MappedUniformBuffers[i].buffer.bufferMemory, 0, bufferSize, 0, &m_MappedUniformBuffers[i].mapped);
		}
	}

	void VulkanRenderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		// TODO
		/* Memory transfer operations are executed using command buffers, just like drawing commands.
		 * Therefore we must first allocate a temporary command buffer.
		 * You may wish to create a separate command pool for these kinds of short-lived buffers, because the implementation may be able to apply memory allocation optimizations.
		 * You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool generation in that case.
		 */

		VkCommandBuffer commandBuffer{ m_CommandPoolManager.BeginSingleTimeCommands() };

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		m_CommandPoolManager.EndSingleTimeCommands(commandBuffer);
	}

	void VulkanRenderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer{ m_CommandPoolManager.BeginSingleTimeCommands() };

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage( commandBuffer,
								buffer,
								image,
								VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								1,
								&region );

		m_CommandPoolManager.EndSingleTimeCommands(commandBuffer);
	}

	void VulkanRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
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
		renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChainContext.GetExtent();

		VkClearColorValue constexpr clearColor{ 0.0f, 0.0f, 0.0f, 0.0f };
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

			VkBuffer vertexBuffers[] { m_VertexBuffer.buffer };
			VkDeviceSize offsets[] { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			// you can only have a single index buffer. It's unfortunately not possible to use different indices for each vertex attribute,
			// so we do still have to completely duplicate vertex data even if just one attribute varies.
			vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			//static_assert(sizeof(m_Indices[0]) == 2);
			static_assert(sizeof(m_Indices[0]) == 4);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.GetPipelineLayout(), 0, 1, &m_DescriptorContext.GetDescriptorSets()[m_CurrentFrame], 0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
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

	void VulkanRenderer::DrawFrame()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		// At the start of the frame, we want to wait until the previous frame has finished, so that the command buffer and semaphores are available to use.
		vkWaitForFences(deviceContext->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult const acquireNextImageResult{ vkAcquireNextImageKHR(deviceContext->GetLogicalDevice(), m_SwapChainContext.GetSwapchain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex) };

		if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_FramebufferResized = false;
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

		UpdateUniformBuffer(m_CurrentFrame);

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
			m_FramebufferResized = false;
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

	void VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage)
	{
		//TODO push constants

		static auto startTime{ std::chrono::high_resolution_clock::now() };

		auto const currentTime{ std::chrono::high_resolution_clock::now() };
		float const time{ std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count() };

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(m_SwapChainContext.GetExtent().width) / static_cast<float>(m_SwapChainContext.GetExtent().width), 0.1f, 10.0f);

		ubo.proj[1][1] *= -1;

		memcpy(m_MappedUniformBuffers[currentImage].mapped, &ubo, sizeof(ubo));
	}

	void VulkanRenderer::RecreateSwapchain()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		// TODO 
		// It is possible to create a new swap chain while drawing commands on an image from the old swap chain are still in - flight.
		// You need to pass the previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR struct and destroy the old swap chain as soon as you've finished using it.


		// This essentially pauses until the window is in the foreground again
		int width{};
		int height{};
		glfwGetFramebufferSize(m_pWindow, &width, &height);

		while (width == 0 || height == 0) 
		{
			glfwGetFramebufferSize(m_pWindow, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(deviceContext->GetLogicalDevice());

		CleanupSwapchain();

		m_SwapChainContext.Initialize(m_pWindow, &m_SurfaceContext);

		CreateColorResources();
		CreateDepthResources();
		CreateFrameBuffers();
	}

	void VulkanRenderer::CreateTextureImage()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		int texWidth{};
		int texHeight{};
		int texChannels{};

		stbi_uc* const pixels{ stbi_load("Textures/VikingRoom.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha) };
		VkDeviceSize const imageSize{ static_cast<uint32_t>(texWidth * texHeight * 4) };

		if (!pixels) 
		{
			throw std::runtime_error("Failed to load texture image!");
		}


		VulkanBuffer stagingBuffer{ imageSize,
									 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		void* data;
		vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory , 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);

		stbi_image_free(pixels);

		m_TextureImage = VulkanImage
		{
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			static_cast<uint32_t>(texWidth),
			static_cast<uint32_t>(texHeight),
			static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1
		};

		m_TextureImage.TransitionImageLayout(this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(stagingBuffer.buffer, m_TextureImage.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		// is transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		m_TextureImage.GenerateMipmaps(this);

		stagingBuffer.Destroy();

		m_TextureImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void VulkanRenderer::CreateTextureSampler()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		// If addressed outside of bounds, repeat (tileable texture)
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(deviceContext->GetPhysicalDevice(), &properties);

		// If we want to go for maximum quality, we can simply use the limit directly
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.f;
		//samplerInfo.minLod = static_cast<float>(m_TextureImage.mipLevels / 2);
		samplerInfo.maxLod = static_cast<float>(m_TextureImage.mipLevels);
		samplerInfo.mipLodBias = 0.0f; // Optional

		if (vkCreateSampler(deviceContext->GetLogicalDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

	void VulkanRenderer::CreateDepthResources()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		VkFormat const depthFormat{ deviceContext->FindDepthFormat() };

		m_DepthImage = VulkanImage
		{
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			deviceContext->GetSampleCount(),
			m_SwapChainContext.GetExtent().width,
			m_SwapChainContext.GetExtent().height
		};

		m_DepthImage.CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
		// No need to transition since we will do this in the render pass.
	}

	void VulkanRenderer::CleanupSwapchain()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		for (auto const& framebuffer : m_SwapChainFramebuffers)
		{
			vkDestroyFramebuffer(deviceContext->GetLogicalDevice(), framebuffer, nullptr);
		}

		m_DepthImage.Destroy();
		m_ColorImage.Destroy();

		m_SwapChainContext.Destroy();
	}

	void VulkanRenderer::CreateGlobalBuffers()
	{
		m_GlobalVertexBuffer = { MAX_VERTEX_BUFFER_SIZE,
									VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		m_GlobalIndexBuffer = { MAX_INDEX_BUFFER_SIZE,
									VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		m_InstanceDataBuffer = { MAX_INSTANCE_BUFFER_SIZE,
									VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
	}

	void VulkanRenderer::CreateColorResources()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkFormat const colorFormat{ m_SwapChainContext.GetImageFormat() };

		m_ColorImage = VulkanImage
		{
			colorFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			deviceContext->GetSampleCount(),
			m_SwapChainContext.GetExtent().width,
			m_SwapChainContext.GetExtent().height
		};

		m_ColorImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
	}

	VulkanRenderer::VulkanImage::VulkanImage(VkFormat imgFormat, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkSampleCountFlagBits numSamples, uint32_t imgWidth, uint32_t imgHeight, uint32_t imgMipLevels)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		format = imgFormat;

		width = imgWidth;
		height = imgHeight;

		mipLevels = imgMipLevels;


		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;

		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = numSamples;
		imageInfo.flags = 0; // Optional

		if (vkCreateImage(deviceContext->GetLogicalDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(deviceContext->GetLogicalDevice(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanUtils::FindMemoryType(deviceContext->GetPhysicalDevice() ,memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(deviceContext->GetLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory!");
		}

		vkBindImageMemory(deviceContext->GetLogicalDevice(), image, imageMemory, 0);
	}

	void VulkanRenderer::VulkanImage::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		for (auto& imageView : imageViews)
		{
			VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), imageView, nullptr);
		}

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), image, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), imageMemory, nullptr);
	}

	void VulkanRenderer::VulkanImage::TransitionImageLayout(VulkanRenderer* pRenderer, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkCommandBuffer const commandBuffer{ pRenderer->m_CommandPoolManager.BeginSingleTimeCommands() };

		VkImageMemoryBarrier barrier{};

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			throw std::invalid_argument("Unsupported layout transition!");
		}

		// TODO
		//• In the tutorial, the transition is done using a transient command
		//	buffer.These are used for one time usage(e.g.generating
		//		data).So, this is obviously not necessary if the transition is part
		//	of the command buffer you are already recording for every
		//	frame(which is not the case when loading textures).
		//	• You can, and should, also track the current layout of an image
		//	yourself!

		// https://docs.vulkan.org/spec/latest/chapters/synchronization.html#synchronization-access-types-supported
		vkCmdPipelineBarrier(commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		pRenderer->m_CommandPoolManager.EndSingleTimeCommands(commandBuffer);
	}

	void VulkanRenderer::VulkanImage::GenerateMipmaps(VulkanRenderer* pRenderer)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkCommandBuffer const commandBuffer{ pRenderer->m_CommandPoolManager.BeginSingleTimeCommands() };

		// TODO
		/*
		 *There are two alternatives in this case.
		 *You could implement a function that searches common texture image formats for one that does support linear blitting,
		 *or you could implement the mipmap generation in software with a library like stb_image_resize.
		 *Each mip level can then be loaded into the image in the same way that you loaded the original image.
		 *
		 *It should be noted that it is uncommon in practice to generate the mipmap levels at runtime anyway.
		 *Usually they are pregenerated and stored in the texture file alongside the base level to improve loading speed.
		 *Implementing resizing in software and loading multiple levels from a file is left as an exercise to the reader.
		 **/

		 // Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(deviceContext->GetPhysicalDevice(), format, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			throw std::runtime_error("Texture image format does not support linear blitting!");
		}

		// We're going to make several transitions, so we'll reuse this VkImageMemoryBarrier
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth{ static_cast<int32_t>(width) };
		int32_t mipHeight{ static_cast<int32_t>(height) };

		for (uint32_t i{ 1 }; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;


			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1)
			{
				mipWidth /= 2;
			}
			if (mipHeight > 1)
			{
				mipHeight /= 2;
			}
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);


		pRenderer->m_CommandPoolManager.EndSingleTimeCommands(commandBuffer);
	}

	uint32_t VulkanRenderer::VulkanImage::CreateImageView(VkImageAspectFlags aspectFlags)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(deviceContext->GetLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture image view!");
		}

		imageViews.emplace_back(imageView);

		return static_cast<uint32_t>(imageViews.size() - 1);
	}
}


