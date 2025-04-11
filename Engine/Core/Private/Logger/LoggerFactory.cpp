#include "CorePCH.h"

#include "Logger/LoggerFactory.h"

#include "ConsoleLogger.h"
#include "FileLogger.h"

namespace MauCor
{
	std::unique_ptr<Logger> CreateConsoleLogger() noexcept
	{
		return std::make_unique<ConsoleLogger>();
	}

	std::unique_ptr<Logger> CreateFileLogger(std::filesystem::path&& filePath) noexcept
	{
		return std::make_unique<FileLogger>(std::move(filePath));
	}
}