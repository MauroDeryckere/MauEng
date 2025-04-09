#ifndef MAUCOR_ASSERTSINTERNAL_H
#define MAUCOR_ASSERTSINTERNAL_H

#include "Asserts/Asserts.h"

namespace MauCor
{
	#define ME_ENGINE_ASSERT(expr, ...) ME_ASSERT_INTERNAL(MauCor::LogCategory::Engine, expr, __VA_ARGS__)
	#define ME_CORE_ASSERT(expr, ...) ME_ASSERT_INTERNAL(MauCor::LogCategory::Core, expr, __VA_ARGS__)
	#define ME_RENDERER_ASSERT(expr, ...) ME_ASSERT_INTERNAL(MauCor::LogCategory::Renderer, expr, __VA_ARGS__)


	#define ME_ENGINE_CHECK(expr, ...) ME_CHECK_INTERNAL(MauCor::LogCategory::Engine, expr, __VA_ARGS__)
	#define ME_CORE_CHECK(expr, ...) ME_CHECK_INTERNAL(MauCor::LogCategory::Core, expr, __VA_ARGS__)
	#define ME_RENDERER_CHECK(expr, ...) ME_CHECK_INTERNAL(MauCor::LogCategory::Renderer, expr, __VA_ARGS__)

	#define ME_ENGINE_VERIFY(expr, ...) ME_VERIFY_INTERNAL(MauCor::LogCategory::Engine, expr, __VA_ARGS__)
	#define ME_CORE_VERIFY(expr, ...) ME_VERIFY_INTERNAL(MauCor::LogCategory::Core, expr, __VA_ARGS__)
	#define ME_RENDERER_VERIFY(expr, ...) ME_VERIFY_INTERNAL(MauCor::LogCategory::Renderer, expr, __VA_ARGS__)
}

#endif