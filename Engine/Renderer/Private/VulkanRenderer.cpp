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
	}

	VulkanRenderer::~VulkanRenderer()
	{
		for (auto const& framebuffer : m_SwapChainFramebuffers) 
		{
			vkDestroyFramebuffer(m_DeviceContext->GetLogicalDevice(), framebuffer, nullptr);
		}
	}

	void VulkanRenderer::Render()
	{
		//TODO
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
}


