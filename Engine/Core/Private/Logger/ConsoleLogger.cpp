#include "CorePCH.h"

#include "ConsoleLogger.h"

#include "Config/EngineConfig.h"

namespace MauCor
{
	void ConsoleLogger::LogInternal(LogPriority priority, std::string_view const category, std::string_view const message)
	{
		std::cout << fmt::format("{}[{}] {}{}{} \n", MauEng::LOG_COLOR_CATEGORY, category, PriorityToColour(priority), message, MauEng::LOG_COLOR_RESET);
	}
}
