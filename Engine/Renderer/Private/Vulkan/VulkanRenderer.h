#ifndef MAUREN_VULKANRENDERER_H
#define MAUREN_VULKANRENDERER_H

#include "RendererPCH.h"

#include "Renderer.h"

#include "VulkanInstanceContext.h"
#include "VulkanSurfaceContext.h"
#include "VulkanDebugContext.h"
#include "VulkanDeviceContext.h"
#include "VulkanDescriptorContext.h"
#include "VulkanSwapchainContext.h"
#include "VulkanGraphicsPipeline.h"

#include "Vertex.h"

// Sources
// https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples#swapchain-image-acquire-and-present

namespace MauRen
{
	class VulkanRenderer final : public Renderer
	{
	public:
		explicit VulkanRenderer(GLFWwindow* pWindow);
		virtual ~VulkanRenderer() override;

		virtual void Render() override;
		virtual void ResizeWindow() override;

		[[nodiscard]] VulkanDeviceContext* GetDeviceContext() const noexcept { return m_DeviceContext.get(); }

		VulkanRenderer(VulkanRenderer const&) = delete;
		VulkanRenderer(VulkanRenderer&&) = delete;
		VulkanRenderer& operator=(VulkanRenderer const&) = delete;
		VulkanRenderer& operator=(VulkanRenderer&&) = delete;

	private:
		// "reference" to the window
		GLFWwindow* m_pWindow;

		std::unique_ptr<VulkanInstanceContext> m_InstanceContext;
		std::unique_ptr<VulkanSurfaceContext> m_SurfaceContext;
		std::unique_ptr<VulkanDebugContext> m_DebugContext;
		std::unique_ptr<VulkanDeviceContext> m_DeviceContext;
		std::unique_ptr< VulkanDescriptorContext> m_DescriptorContext;
		std::unique_ptr<VulkanSwapchainContext> m_SwapChainContext;
		std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipeline;

		// no unique ptr owned contxt for this currently, can be moved if the logic grows a lot
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		// no unique ptr owned contxt for this currently, can be moved if the logic grows a lot
		VkCommandPool m_CommandPool;
		// Automatically freed when their pool is destroyed
		std::vector<VkCommandBuffer> m_CommandBuffers;

		// Signal that an image has been acquired from the swapchain and is ready for rendering
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		
		// Signal that rendering has finished and presentation can happen
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;

		// Fence to make sure only one frame is rendering at a time
		std::vector<VkFence> m_InFlightFences;

		uint32_t m_CurrentFrame{ 0 };

		bool m_FramebufferResized{ false };

		struct VulkanBuffer final
		{
			VkBuffer buffer {VK_NULL_HANDLE};
			VkDeviceMemory bufferMemory{ VK_NULL_HANDLE };
			VkDeviceSize size{ 0 };
		};
		// Using a fixed size for now to test the system, until moving on to use the allocator
		const VkDeviceSize MAX_VERTEX_BUFFER_SIZE{ 64 * 1024 * 1024 }; // 64MB
		const VkDeviceSize MAX_INDEX_BUFFER_SIZE{ 32 * 1024 * 1024 };  // 32MB
		const VkDeviceSize MAX_INSTANCE_BUFFER_SIZE{ 16 * 1024 * 1024 }; // 16MB
		//const VkDeviceSize MAX_MESH_DATA_SIZE = 1024 * sizeof(MeshData); // 1024 meshes
		//const VkDeviceSize MAX_DRAW_COMMANDS = 1024 * sizeof(DrawCommand); // 1024 draw calls

		VulkanBuffer m_GlobalVertexBuffer{};
		VulkanBuffer m_GlobalIndexBuffer{};
		VulkanBuffer m_InstanceDataBuffer{};  // Holds per-instance data

		struct VulkanMappedBuffer final
		{
			VulkanBuffer buffer{  };
			void* mapped{ nullptr };
		};
		std::vector<VulkanMappedBuffer> m_MappedUniformBuffers;

		// TODO make a mesh
		VulkanBuffer m_VertexBuffer;
		VulkanBuffer m_IndexBuffer;

		// Temporary
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		// TODO model -> push constant
		struct UniformBufferObject final
		{
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

		struct VulkanImage final
		{
			VkImage image{ VK_NULL_HANDLE };
			VkDeviceMemory imageMemory{ VK_NULL_HANDLE };
			VkFormat format{ VK_FORMAT_UNDEFINED };

			// Currently only work with one view but may support multiple later
			std::vector<VkImageView> imageViews{ };

			uint32_t width{ 0 };
			uint32_t height{ 0 };
			uint32_t mipLevels{ 1 };

			VulkanImage() = default;
			VulkanImage(VulkanRenderer* pRenderer, VkFormat imgFormat, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkSampleCountFlagBits numSamples, uint32_t imgWidth, uint32_t imgHeight, uint32_t imgMipLevels = 1);

			void Destroy(VulkanDeviceContext* pDeviceContext);
			void TransitionImageLayout(VulkanRenderer* pRenderer, VkImageLayout oldLayout, VkImageLayout newLayout);
			void GenerateMipmaps(VulkanRenderer* pRenderer);
			uint32_t CreateImageView(VulkanRenderer* pRenderer, VkImageAspectFlags aspectFlags);
		};

		void CreateTextureImage();

		VulkanImage m_TextureImage{};
		VulkanImage m_DepthImage{};
		VulkanImage m_ColorImage{};

		// Should be managedin e.g a texturemanager
		VkSampler m_TextureSampler{ VK_NULL_HANDLE };

		void CreateFrameBuffers();

		void CreateCommandPool();
		void CreateVertexBuffer();
		void CreateIndexBuffer();

		void CreateCommandBuffers();

		void CreateUniformBuffers();

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void DestroyBuffer(VulkanBuffer const& buffer);

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void CreateSyncObjects();

		void DrawFrame();
		void UpdateUniformBuffer(uint32_t currentImage);

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void CreateTextureSampler();

		//TODO abstract Command away in a struct with a Destroy thats basicallyRAII
		static VkCommandBuffer BeginSingleTimeCommands(VulkanDeviceContext* pDeviceContext, VkCommandPool commandPool);
		static void EndSingleTimeCommands(VulkanDeviceContext* pDeviceContext, VkCommandPool commandPool, VkCommandBuffer commandBuffer);


		void CreateColorResources();
		void CreateDepthResources();

		void RecreateSwapchain();
		void CleanupSwapchain();

		void CreateGlobalBuffers();
	};
}

#endif
