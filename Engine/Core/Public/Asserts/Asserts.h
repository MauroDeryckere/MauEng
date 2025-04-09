#ifndef MAUCOR_ASSERTS_H
#define MAUCOR_ASSERTS_H

#include <iostream>
#include <cstdlib>

#if defined(_MSC_VER)
	#define DEBUG_BREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
	#define DEBUG_BREAK() __builtin_trap()
#else
	#define DEBUG_BREAK() std::abort()
#endif

namespace MauCor
{
	void InternalAssert(const char* expression, const char* file, int line, const char* message = nullptr);

#ifdef _DEBUG
#define ME_ASSERT(expr) \
        do { \
            if (!(expr)) { \
                MauCor::InternalAssert(#expr, __FILE__, __LINE__); \
                DEBUG_BREAK(); \
            } \
        } while (0)

#define ME_ASSERT_MSG(expr, message) \
        do { \
            if (!(expr)) { \
                MauCor::InternalAssert(#expr, __FILE__, __LINE__, message); \
                DEBUG_BREAK(); \
            } \
        } while (0)
#else
	#define ME_ASSERT(expr) void(0)
	#define ME_ASSERT_MSG(expr , message) void(0)
#endif
}

#endif