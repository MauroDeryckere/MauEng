#ifndef MAUCOR_ASSERTSINTERNAL_H
#define MAUCOR_ASSERTSINTERNAL_H

#include "Asserts/Asserts.h"

namespace MauCor
{
	#define ME_ENGINE_ASSERT(expr, ...) ME_ASSERT_INTERNAL(LogEngine, expr, __VA_ARGS__)
	#define ME_CORE_ASSERT(expr, ...) ME_ASSERT_INTERNAL(LogCore, expr, __VA_ARGS__)
	#define ME_RENDERER_ASSERT(expr, ...) ME_ASSERT_INTERNAL(LogRenderer, expr, __VA_ARGS__)


	#define ME_ENGINE_CHECK(expr, ...) ME_CHECK_INTERNAL(LogEngine, expr, __VA_ARGS__)
	#define ME_CORE_CHECK(expr, ...) ME_CHECK_INTERNAL(LogCore, expr, __VA_ARGS__)
	#define ME_RENDERER_CHECK(expr, ...) ME_CHECK_INTERNAL(LogRenderer, expr, __VA_ARGS__)

	#define ME_ENGINE_VERIFY(expr, ...) ME_VERIFY_INTERNAL(LogEngine, expr, __VA_ARGS__)
	#define ME_CORE_VERIFY(expr, ...) ME_VERIFY_INTERNAL(LogCore, expr, __VA_ARGS__)
	#define ME_RENDERER_VERIFY(expr, ...) ME_VERIFY_INTERNAL(LogRenderer, expr, __VA_ARGS__)
}

#endif