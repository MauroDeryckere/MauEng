#ifndef MAUCOR_FILELOGGER_H
#define MAUCOR_FILELOGGER_H

#include "Logger.h"

#include <filesystem>
namespace MauCor
{
	// No functuionality yet, TODO
	class FileLogger final : public Logger
	{
	public:
		explicit FileLogger(std::filesystem::path const& path);
		virtual ~FileLogger() override = default;

		FileLogger(FileLogger const&) = delete;
		FileLogger(FileLogger&&) = delete;
		FileLogger& operator=(FileLogger const&) = delete;
		FileLogger& operator=(FileLogger&&) = delete;

	private:
		std::filesystem::path m_LogFile{ };

		void LogInternal(LogPriority priority, LogCategory category, std::string const& message) override;
	};
}

#endif