#ifndef MAUCOR_ASSERTS_H
#define MAUCOR_ASSERTS_H

#include <iostream>
#include <cstdlib>

#include "Logger/LogCategories.h"
#include "Config/EngineConfig.h"

namespace MauCor
{
	void InternalAssert(LogCategory category, char const* expression, char const* file, int line, char const* message = nullptr) noexcept;

#pragma region Macros
#pragma region MacrosInternal
	// What should we trigger on assert failure (specifically in debug)
	#if defined(_MSC_VER)
		#define DEBUG_BREAK() __debugbreak()
	#elif defined(__clang__) || defined(__GNUC__)
		#define DEBUG_BREAK() __builtin_trap()
	#else
		#define DEBUG_BREAK() std::abort()
	#endif

	#ifdef ENABLE_ASSERTS
		// Internal assert
		#define ME_ASSERT_INTERNAL(category, expr, ...) \
		        do { \
		            if (!(expr)) { \
		                MauCor::InternalAssert(category, #expr, __FILE__, __LINE__, __VA_ARGS__); \
		                DEBUG_BREAK(); \
		            } \
		        } while (0)
	#else
		#define ME_ASSERT_INTERNAL(category, expr, ...)
	#endif
#pragma endregion

	#define ME_ENGINE_ASSERT(expr, ...) ME_ASSERT_INTERNAL(MauCor::LogCategory::Engine, expr, __VA_ARGS__)
	#define ME_CORE_ASSERT(expr, ...) ME_ASSERT_INTERNAL(MauCor::LogCategory::Core, expr, __VA_ARGS__)
	#define ME_RENDERER_ASSERT(expr, ...) ME_ASSERT_INTERNAL(MauCor::LogCategory::Renderer, expr, __VA_ARGS__)

	#define ME_ASSERT(expr, ...) ME_ASSERT_INTERNAL(MauCor::LogCategory::Game, expr, __VA_ARGS__)
#pragma endregion
}

#endif