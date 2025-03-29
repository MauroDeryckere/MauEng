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

		m_SwapChainContext.InitializeResourcesAndCreateFrames(&m_GraphicsPipeline);

		CreateTextureImage();
		CreateTextureSampler();


		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		Utils::LoadModel("Models/VikingRoom.obj", vertices, indices);

		constexpr size_t NUM_MESHES{ 2 };

		m_Meshes.resize(NUM_MESHES);
		m_Meshes[0].Initialize(m_CommandPoolManager, vertices, indices);
		m_Meshes[0].m_PushConstants.m_ModelMatrix = glm::translate(m_Meshes[0].m_PushConstants.m_ModelMatrix, { -2, 0.f, 0.f});
		m_Meshes[0].m_PushConstants.m_AlbedoTextureID = 0;

		std::vector<Vertex> vertices2;
		std::vector<uint32_t> indices2;
		Utils::LoadModel("Models/Skull.obj", vertices2, indices2);

		m_Meshes[1].Initialize(m_CommandPoolManager, vertices2, indices2);
	//	m_Meshes[1].m_PushConstants.m_ModelMatrix = glm::rotate(m_Meshes[1].m_PushConstants.m_ModelMatrix, glm::radians(90.f), {1,0,0});
		m_Meshes[1].m_PushConstants.m_ModelMatrix = glm::scale(m_Meshes[1].m_PushConstants.m_ModelMatrix, { .2, .2, .2 });
		m_Meshes[1].m_PushConstants.m_ModelMatrix = glm::translate(m_Meshes[1].m_PushConstants.m_ModelMatrix, {0, 15,-2 });
		m_Meshes[1].m_PushConstants.m_AlbedoTextureID = 1;
		CreateUniformBuffers();

		//CreateGlobalBuffers();

		m_DescriptorContext.CreateDescriptorPool();

		std::vector<VulkanBuffer> tempUniformBuffers;
		for (auto const& b : m_MappedUniformBuffers)
		{
			tempUniformBuffers.emplace_back(b.buffer);
		}
		std::vector<VkImageView> views;
		for (auto v : m_TextureImage.imageViews)
		{
			views.emplace_back(v);
		}
		for (auto v : m_TextureImage2.imageViews)
		{
			views.emplace_back(v);
		}

		
		m_DescriptorContext.CreateDescriptorSets(tempUniformBuffers,
												0, 
												sizeof(UniformBufferObject), 
												VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
												views, 
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

		for (auto& mesh : m_Meshes)
		{
			mesh.Destroy();
		}

		vkDestroySampler(deviceContext->GetLogicalDevice(), m_TextureSampler, nullptr);

		m_TextureImage.Destroy();
		m_TextureImage2.Destroy();

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

	void VulkanRenderer::Render()
	{
		DrawFrame();
	}

	void VulkanRenderer::ResizeWindow()
	{
		m_FramebufferResized = true;
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

			// you can only have a single index buffer. It's unfortunately not possible to use different indices for each vertex attribute,
			// so we do still have to completely duplicate vertex data even if just one attribute varies.

			// This is not at all an optimal approach, and will be refactored later. This is simply to test spawning & renderingmultiple meshes while refactoring
			for (auto& mesh : m_Meshes)
			{
				//TODO move to some form of an update later
				static auto startTime{ std::chrono::high_resolution_clock::now() };

				auto const currentTime{ std::chrono::high_resolution_clock::now() };
				float const deltaTime{ std::chrono::duration<float>(currentTime - startTime).count() };
				startTime = currentTime; // Update start time for the next frame

				float rotationSpeed = glm::radians(90.0f); // 90 degrees per second


				glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotationSpeed * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f));
				mesh.m_PushConstants.m_ModelMatrix *= rotation;

				vkCmdPushConstants(commandBuffer, m_GraphicsPipeline.GetPipelineLayout(),
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mesh.m_PushConstants),
					&mesh.m_PushConstants);

				auto const idxCount{ mesh.Draw(commandBuffer) };


				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.GetPipelineLayout(), 0, 1, &m_DescriptorContext.GetDescriptorSets()[m_CurrentFrame], 0, nullptr);

				vkCmdDrawIndexed(commandBuffer, idxCount, 1, 0, 0, 0);
			}
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
		UniformBufferObject ubo{};

		ubo.view = glm::lookAt(glm::vec3(0, -8, 3), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		float const aspectRatio{ static_cast<float>(m_SwapChainContext.GetExtent().width) / static_cast<float>(m_SwapChainContext.GetExtent().height) };
		ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);

		// Flip Y-axis for Vulkan coordinate system
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

		m_SwapChainContext.ReCreate(m_pWindow, &m_GraphicsPipeline, &m_SurfaceContext);
	}

	void VulkanRenderer::CreateTextureImage()
	{
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
			vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, imageSize, 0, &data);
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

			m_TextureImage.TransitionImageLayout(m_CommandPoolManager, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			VulkanBuffer::CopyBufferToImage(m_CommandPoolManager, stagingBuffer.buffer, m_TextureImage.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
			// is transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

			m_TextureImage.GenerateMipmaps(m_CommandPoolManager);

			stagingBuffer.Destroy();

			m_TextureImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		}

		{
			auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

			int texWidth{};
			int texHeight{};
			int texChannels{};

			stbi_uc* const pixels{ stbi_load("Textures/Skull.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha) };
			VkDeviceSize const imageSize{ static_cast<uint32_t>(texWidth * texHeight * 4) };

			if (!pixels)
			{
				throw std::runtime_error("Failed to load texture image!");
			}


			VulkanBuffer stagingBuffer{ imageSize,
										 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
										 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

			void* data;
			vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
			vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);

			stbi_image_free(pixels);

			m_TextureImage2 = VulkanImage
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

			m_TextureImage2.TransitionImageLayout(m_CommandPoolManager, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			VulkanBuffer::CopyBufferToImage(m_CommandPoolManager, stagingBuffer.buffer, m_TextureImage2.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
			// is transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

			m_TextureImage2.GenerateMipmaps(m_CommandPoolManager);

			stagingBuffer.Destroy();

			m_TextureImage2.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		}
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

	void VulkanRenderer::VulkanMesh::Initialize(VulkanCommandPoolManager const& CmdPoolManager, std::vector<Vertex> const& vertices, std::vector<uint32_t> const& indices)
	{
		m_IndexCount = indices.size();
		m_VertexCount = vertices.size();

		CreateVertexBuffer(CmdPoolManager, vertices);
		CreateIndexBuffer(CmdPoolManager, indices);
	}

	void VulkanRenderer::VulkanMesh::Destroy()
	{
		m_IndexBuffer.Destroy();
		m_VertexBuffer.Destroy();
	}

	uint32_t VulkanRenderer::VulkanMesh::Draw(VkCommandBuffer commandBuffer) const
	{
		VkBuffer vertexBuffers[] = { m_VertexBuffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		// Bind the index buffer
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		return m_IndexCount;
	}

	void VulkanRenderer::VulkanMesh::CreateVertexBuffer(VulkanCommandPoolManager const& CmdPoolManager, std::vector<Vertex> const& vertices)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize const bufferSize{ sizeof(vertices[0]) * vertices.size() };

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

		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);

		m_VertexBuffer = { bufferSize,
							VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

		VulkanBuffer::CopyBuffer(CmdPoolManager, stagingBuffer.buffer, m_VertexBuffer.buffer, bufferSize);

		stagingBuffer.Destroy();
	}

	void VulkanRenderer::VulkanMesh::CreateIndexBuffer(VulkanCommandPoolManager const& CmdPoolManager, std::vector<uint32_t> const& indices)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize const bufferSize{ sizeof(indices[0]) * indices.size() };

		VulkanBuffer stagingBuffer{ bufferSize,
									VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		void* data;
		vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);


		m_IndexBuffer = { bufferSize,
							 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

		VulkanBuffer::CopyBuffer(CmdPoolManager, stagingBuffer.buffer, m_IndexBuffer.buffer, bufferSize);

		stagingBuffer.Destroy();
	}
}


