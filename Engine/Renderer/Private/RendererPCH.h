#ifndef MAUREN_RENDERER_PCH
#define MAUREN_RENDERER_PCH

#include <iostream>

#include "Utils.h"

#include <string>
#include <cstdint>

#include <vector>
#include <array>
#include <span>
#include <set>

#include <memory>

#include <stdexcept>
#include <cassert>

#include <algorithm>
#include <optional>

// do this using macros in the future if more than 1 renderer support
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "../Config/VulkanConfig.h"

#include "../../Engine/Public/Config/EngineConfig.h"

#endif