#ifndef MAUREN_RENDERER_PCH
#define MAUREN_RENDERER_PCH

#define USING_VULKAN_RENDERER 1

#include "CorePCH.h"

#include "AssertsInternal.h"

#ifdef USING_VULKAN_RENDERER
	#include <SDL3/SDL.h>
	#include <SDL3/SDL_vulkan.h>

	#include <vulkan/vulkan_core.h>

	#include "../Config/VulkanConfig.h"

	#include "Config/EngineConfig.h"
	#include "Vulkan/VulkanUtils.h"

	#include "Vulkan/VulkanDeviceContextManager.h"

	#include "vk_mem_alloc.h"

#endif

#endif