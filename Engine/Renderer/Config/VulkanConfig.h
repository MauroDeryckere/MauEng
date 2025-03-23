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
		"VK_LAYER_KHRONOS_validation",
	};

	std::vector<char const*> const DEVICE_EXTENSIONS
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	uint32_t constexpr MAX_FRAMES_IN_FLIGHT{ 2 };
	static_assert(MAX_FRAMES_IN_FLIGHT > 0);

	// Debug output colours

	// White (default)
	static auto constexpr colorReset{ "\033[0m" };
	// Red
	static auto constexpr colorError{ "\033[1;31m" };
	// Yellow
	static auto constexpr colorWarning{ "\033[1;33m" };
	// Cyan
	static auto constexpr colorInfo{ "\033[1;36m" };
	// Gray
	static auto constexpr colorGeneral{ "\033[1;90m" };
	// Blue
	static auto constexpr colorCategory{ "\033[1;34m" };

}

#endif // MAUREN_VULKANCONFIG_H