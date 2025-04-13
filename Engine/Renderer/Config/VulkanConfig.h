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
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
	};

	uint32_t constexpr MAX_FRAMES_IN_FLIGHT{ 3 };
	static_assert(MAX_FRAMES_IN_FLIGHT > 0);

	uint32_t constexpr MAX_TEXTURES{ 1024 };
	bool constexpr DEBUG_OUT_MAT{ true };
}

#endif // MAUREN_VULKANCONFIG_H