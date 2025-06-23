#include "Asserts/Asserts.h"

#include "CoreServiceLocator.h"

namespace MauCor
{
	void InternalAssert(ELogCategory category, char const* expression, char const* file, int line, char const* message) noexcept
	{
		ME_LOG_FATAL(category, "Assertion failed: {} File: {}, Line: {}, Message: {}", expression, file, line, message ? message : "NO MESSAGE SPECIFIED");
	}

	void InternalCheck(ELogCategory category, char const* expression, char const* file, int line, char const* message) noexcept
	{
		ME_LOG_FATAL(category, "Check failed: {} File: {}, Line: {}, Message: {}", expression, file, line, message ? message : "NO MESSAGE SPECIFIED");
	}

	void InternalVerify(ELogCategory category, char const* expression, char const* file, int line, char const* message) noexcept
	{
		ME_LOG_WARN(category, "Verify failed: {} File: {}, Line: {}, Message: {}", expression, file, line, message ? message : "NO MESSAGE SPECIFIED");
	}
}
