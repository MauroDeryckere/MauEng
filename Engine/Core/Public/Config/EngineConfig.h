#ifndef MAUENG_ENGINECONFIG_H
#define MAUENG_ENGINECONFIG_H

namespace MauEng
{
	// Debug output colours
	// White (default)
	auto constexpr LOG_COLOR_RESET{ "\033[0m" };
	// Blue
	auto constexpr LOG_COLOR_CATEGORY{ "\033[1;34m" };

	auto constexpr LOG_COLOR_TRACE{ "\033[1;37m" };
	auto constexpr LOG_COLOR_INFO{ "\033[1;32m" };
	auto constexpr LOG_COLOR_DEBUG{ "\033[1;36m" };
	auto constexpr LOG_COLOR_WARNING{ "\033[1;33m" };
	auto constexpr LOG_COLOR_ERROR{ "\033[1;31m" };
	auto constexpr LOG_COLOR_FATAL{ "\033[1;31m" };


//#define SHIPPING

#ifndef SHIPPING
	bool constexpr ENABLE_DEBUG_RENDERING{ true };
	bool constexpr LOG_TO_FILE{ false };
#else
	bool constexpr ENABLE_DEBUG_RENDERING{ false };
	bool constexpr LOG_TO_FILE{ true };
#endif

  uint32_t constexpr MAX_FILE_SIZE_BEFORE_ROTATE{ 5'000 };

#if _DEBUG
	#define ENABLE_ASSERTS
#endif

#ifndef SHIPPING
	#define ENABLE_PROFILER
#endif

#ifdef ENABLE_PROFILER
  uint32_t constexpr NUM_FRAMES_TO_PROFILE{ 2 };
#endif

	bool constexpr LIMIT_FPS{ false };
	bool constexpr LOG_FPS{ true };
}

#endif