#include "Asserts/Asserts.h"

#include "CoreServiceLocator.h"

namespace MauCor
{
	void InternalAssert(const char* expression, const char* file, int line, const char* message)
	{
		ME_LOG_FATAL(LogCategory::Core, "Assertion failed: {} File: {}, Line: {}, Message: {}", expression, file, line, message ? message : "NO MESSAGE SPECIFIED");
	}
}
