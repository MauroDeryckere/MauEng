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


#define ENABLE_FILE_LOGGING 0
#define ENABLE_DEBUG_RENDERING 0
#define ENABLE_ASSERTS 0

#define ENABLE_PROFILER 0
#define	USE_OPTICK_LIBRARY 0

#ifdef MAUENG_LOG_TO_FILE
	#undef ENABLE_FILE_LOGGING
	#define ENABLE_FILE_LOGGING 1
#endif

#ifdef MAUENG_ENABLE_DEBUG_RENDERING
	#undef ENABLE_DEBUG_RENDERING
	#define ENABLE_DEBUG_RENDERING 1
#endif

#ifdef MAUENG_ENABLE_ASSERTS
	#undef ENABLE_ASSERTS
	#define ENABLE_ASSERTS 1
#endif

#ifdef MAUENG_ENABLE_PROFILER
	#undef ENABLE_PROFILER
	#define ENABLE_PROFILER 1
#endif

#if ENABLE_PROFILER
	uint32_t constexpr NUM_FRAMES_TO_PROFILE{ 5 };

	// Toggle using ME profiler (w/ google://tracing) or optick library
	#ifdef MAUENG_USE_OPTICK
		#undef USE_OPTICK_LIBRARY
		#define USE_OPTICK_LIBRARY 1
	#endif

	#if USE_OPTICK_LIBRARY
		#define USE_OPTICK 1
	#else
		#define USE_OPTICK 0
	#endif
#endif

	bool constexpr LIMIT_FPS{ true };
	bool constexpr LOG_FPS{ true };
}

#endif