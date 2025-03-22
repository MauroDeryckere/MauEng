#include "VulkanRenderer.h"

namespace MauRen
{
	VulkanRenderer::VulkanRenderer(GLFWwindow* pWindow) :
		Renderer{ pWindow },
		m_InstanceContext{ std::make_unique<VulkanInstanceContext>() },
		m_SurfaceContext{ std::make_unique<VulkanSurfaceContext>(m_InstanceContext.get(), pWindow) },
		m_DebugContext{ std::make_unique<VulkanDebugContext>(m_InstanceContext.get()) },
		m_DeviceContext{ std::make_unique<VulkanDeviceContext>(m_SurfaceContext.get(), m_InstanceContext.get())},
		m_SwapChainContext{ std::make_unique<VulkanSwapchainContext>(pWindow, m_SurfaceContext.get(), m_DeviceContext.get()) },
		m_GraphicsPipeline{ std::make_unique<VulkanGraphicsPipeline>(m_DeviceContext.get(), m_SwapChainContext.get() ) }
	{
		CreateFrameBuffers();
		CreateCommandPool();
		CreateCommandBuffer();
		CreateSyncObjects();
	}

	VulkanRenderer::~VulkanRenderer()
	{
		// Wait for GPU to finish everything
		vkDeviceWaitIdle(m_DeviceContext->GetLogicalDevice());

		vkDestroySemaphore(m_DeviceContext->GetLogicalDevice(), m_ImageAvailableSemaphore, nullptr);
		vkDestroySemaphore(m_DeviceContext->GetLogicalDevice(), m_RenderFinishedSemaphore, nullptr);
		vkDestroyFence(m_DeviceContext->GetLogicalDevice(), m_InFlightFence, nullptr);

		vkDestroyCommandPool(m_DeviceContext->GetLogicalDevice(), m_CommandPool, nullptr);

		for (auto const& framebuffer : m_SwapChainFramebuffers) 
		{
			vkDestroyFramebuffer(m_DeviceContext->GetLogicalDevice(), framebuffer, nullptr);
		}
	}

	void VulkanRenderer::Render()
	{
		DrawFrame();
	}

	void VulkanRenderer::CreateFrameBuffers()
	{
		auto const& imageViews{ m_SwapChainContext->GetImageViews() };

		m_SwapChainFramebuffers.resize(imageViews.size());

		for (size_t i{0}; i < imageViews.size(); ++i)
		{
			VkImageView const attachments[] { imageViews[i]};
			
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_GraphicsPipeline->GetRenderPass();
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_SwapChainContext->GetExtent().width;
			framebufferInfo.height = m_SwapChainContext->GetExtent().height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_DeviceContext->GetLogicalDevice(), &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}
	}

	void VulkanRenderer::CreateCommandPool()
	{
		// Record commands for drawing, which is why we've chosen the graphics queue family
		QueueFamilyIndices const queueFamilyIndices{ m_DeviceContext->FindQueueFamilies() };

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(m_DeviceContext->GetLogicalDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool!");
		}
	}

	void VulkanRenderer::CreateCommandBuffer()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		
		// VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
		// VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		allocInfo.commandBufferCount = 1;
			

		if (vkAllocateCommandBuffers(m_DeviceContext->GetLogicalDevice(), &allocInfo, &m_CommandBuffer) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}
	}

	void VulkanRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};

		// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
		// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
		// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : The command buffer can be resubmitted while it is also already pending execution.
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional; only relevant for secondary

		//Note: if the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset it.
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_GraphicsPipeline->GetRenderPass();
		renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChainContext->GetExtent();

		VkClearValue constexpr clearColor {0.0f, 0.0f, 0.0f, 1.0f} ;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->GetPipeline() );

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_SwapChainContext->GetExtent().width);
		viewport.height = static_cast<float>(m_SwapChainContext->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_SwapChainContext->GetExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);


		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to record command buffer!");
		}
	}

	void VulkanRenderer::CreateSyncObjects()
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// Create the fence in the signaled state, so that the first call to vkWaitForFences() returns immediately since the fence is already signaled
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


		if (vkCreateSemaphore(m_DeviceContext->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore) != VK_SUCCESS 
		 || vkCreateSemaphore(m_DeviceContext->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore) != VK_SUCCESS 
		 || vkCreateFence(m_DeviceContext->GetLogicalDevice(), &fenceInfo, nullptr, &m_InFlightFence) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create semaphores!");
		}
	}

	void VulkanRenderer::DrawFrame()
	{
		// At the start of the frame, we want to wait until the previous frame has finished, so that the command buffer and semaphores are available to use.
		vkWaitForFences(m_DeviceContext->GetLogicalDevice(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(m_DeviceContext->GetLogicalDevice(), 1, &m_InFlightFence);

		uint32_t imageIndex;
		//TODO error handling
		vkAcquireNextImageKHR(m_DeviceContext->GetLogicalDevice(), m_SwapChainContext->GetSwapchain(), UINT64_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		vkResetCommandBuffer(m_CommandBuffer, 0);
		RecordCommandBuffer(m_CommandBuffer, imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore const waitSemaphores[] { m_ImageAvailableSemaphore };
		VkPipelineStageFlags const waitStages[] { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

		VkSemaphore const signalSemaphores[] { m_RenderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// m_InFlightFence here effectively means, this submit must be finished before our next render may start
		if (vkQueueSubmit(m_DeviceContext->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFence) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		// The first two parameters specify which semaphores to wait on before presentation can happen
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR const swapChains[] { m_SwapChainContext->GetSwapchain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		// Not necessary when using a single swapchain
		presentInfo.pResults = nullptr; // Optional

		//TODO error handling
		vkQueuePresentKHR(m_DeviceContext->GetPresentQueue(), &presentInfo);
	}
}


