#include "ConsoleLogger.h"

namespace MauCor
{
	void ConsoleLogger::LogInternal(LogPriority priority, std::string const& message)
	{
		std::cout << fmt::format("Log Priority: {}, Message: {}\n", static_cast<int>(priority), message);
	}
}
