#include "CorePCH.h"

#include "ConsoleLogger.h"

#include "Config/EngineConfig.h"

namespace MauCor
{
	void ConsoleLogger::LogInternal(LogPriority priority, LogCategory category, std::string const& message)
	{
		std::cout << fmt::format("{}[{}] {}{}{} \n", MauEng::LOG_COLOR_CATEGORY, CategoryToString(category), PriorityToColour(priority), message, MauEng::LOG_COLOR_RESET);
	}
}
