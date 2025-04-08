#ifndef MAUCOR_LOGGERFACTORY_H
#define MAUCOR_LOGGERFACTORY_H

#include <filesystem>

namespace MauCor
{
	class Logger;

	[[nodiscard]] std::unique_ptr<Logger> CreateConsoleLogger() noexcept;
	[[nodiscard]] std::unique_ptr<Logger> CreateFileLogger(std::filesystem::path&& filePath) noexcept; 
}

#endif