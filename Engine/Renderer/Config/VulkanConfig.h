#ifndef MAUREN_VULKANCONFIG_H
#define MAUREN_VULKANCONFIG_H

// Would be cleaner when moved to some other place than just a header but this is easier for now, can refactor later (// TODO)

namespace MauRen
{
	#ifdef NDEBUG
		bool constexpr ENABLE_VULKAN_VALIDATION_LAYERS{ false };
	#else
		bool constexpr ENABLE_VULKAN_VALIDATION_LAYERS{ true };
	#endif

	// Using a unified queue may result in less overhead and a performance boost
	bool constexpr FORCE_SEPARATE_GRAPHICS_PRESENT_QUEUES{ false };

	bool constexpr AUTO_SELECT_PHYSICAL_DEVICE{ true };

	std::vector<char const*> const VULKAN_VALIDATION_LAYERS
	{
		"VK_LAYER_KHRONOS_validation"
	};
	
	std::vector<char const*> const DEVICE_EXTENSIONS
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		"VK_KHR_create_renderpass2",
		VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		"VK_KHR_synchronization2"
	};
	
	uint32_t constexpr MAX_FRAMES_IN_FLIGHT{ 3 };
	static_assert(MAX_FRAMES_IN_FLIGHT > 0);

	// The descriptor pool creation will throw if this number is too large
	uint32_t constexpr MAX_TEXTURES{ 4'048 };			// For texture array

	uint32_t constexpr MAX_SHADOW_MAPS{ 128 };			// For texture array


	// These are ust arbitrary numbers for now, they have to actally be calculated in the futrue
	uint32_t constexpr MAX_MATERIALS{ 512 };			// Matches MaterialData[] buffer

	uint32_t constexpr MAX_MESH_INSTANCES{ 5'000'000 };	// Matches MeshInstanceData[] buffer
	uint32_t constexpr MAX_MESHES{ 4'048 };				// Matches MeshData[] buffer

	uint32_t constexpr MAX_DRAW_COMMANDS{ 20'000 };		// Matches DrawCommand[] buffer

	uint32_t constexpr MAX_VERTICES{ 10'000'000 };      // Maximum number of vertices (for all meshes)
	uint32_t constexpr MAX_INDICES{ 20'000'000 };       // Maximum number of indices (for all meshes)

	bool constexpr DEBUG_OUT_MAT{ true };

	uint32_t constexpr MAX_LIGHTS{ 128 };

	uint32_t constexpr SHADOW_MAP_SIZE{ 1024 * 4 };
}

#endif // MAUREN_VULKANCONFIG_H