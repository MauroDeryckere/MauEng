#ifndef MAUCOR_ASSERTS_H
#define MAUCOR_ASSERTS_H

#include <iostream>
#include <cstdlib>

#include "Logger/LogCategories.h"
#include "Config/EngineConfig.h"

namespace MauCor
{
	void InternalAssert(LogCategory const& category, char const* expression, char const* file, int line, char const* message = nullptr) noexcept;
	void InternalCheck(LogCategory const& category, char const* expression, char const* file, int line, char const* message = nullptr) noexcept;
	void InternalVerify(LogCategory const& category, char const* expression, char const* file, int line, char const* message = nullptr) noexcept;

#pragma region Macros
#pragma region MacrosInternal
	// What should we trigger on failure
	#if defined(_MSC_VER)
		#define DEBUG_BREAK() __debugbreak()
	#elif defined(__clang__) || defined(__GNUC__)
		#define DEBUG_BREAK() __builtin_trap()
	#else
		#define DEBUG_BREAK() std::abort()
	#endif

	#if ENABLE_ASSERTS
		#define VERIFY_BREAK() DEBUG_BREAK()
	#else
		#define VERIFY_BREAK()
	#endif

	#if ENABLE_ASSERTS
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

	// Internal check
	#define ME_CHECK_INTERNAL(category, expr, ...) \
			do { \
	            if (!(expr)) { \
	                MauCor::InternalCheck(category, #expr, __FILE__, __LINE__, __VA_ARGS__); \
	                DEBUG_BREAK(); \
	            } \
		    } while (0)

	// Internal verify
	#define ME_VERIFY_INTERNAL(category, expr, ...) \
			do { \
	            if (!(expr)) { \
	                MauCor::InternalVerify(category, #expr, __FILE__, __LINE__, __VA_ARGS__); \
					VERIFY_BREAK(); \
	            } \
		    } while (0)

#pragma endregion
	#define ME_ASSERT(expr, ...) ME_ASSERT_INTERNAL(LogGame, expr, __VA_ARGS__)
	#define ME_CHECK(expr, ...) ME_CHECK_INTERNAL(LogGame, expr, __VA_ARGS__)
	#define ME_VERIFY(expr, ...) ME_VERIFY_INTERNAL(LogGame, expr, __VA_ARGS__)
#pragma endregion
}

#endif