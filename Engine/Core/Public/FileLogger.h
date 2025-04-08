#ifndef MAUCOR_FILELOGGER_H
#define MAUCOR_FILELOGGER_H

#include "Logger.h"
#include <fstream>
#include <iostream>
#include <filesystem>
namespace MauCor
{
	// No functuionality yet, TODO
	class FileLogger final : public Logger
	{
	public:
		explicit FileLogger(std::filesystem::path&& path);
		virtual ~FileLogger() override;

		FileLogger(FileLogger const&) = delete;
		FileLogger(FileLogger&&) = delete;
		FileLogger& operator=(FileLogger const&) = delete;
		FileLogger& operator=(FileLogger&&) = delete;

	private:
		std::filesystem::path m_LogFilePath{ };
		std::ofstream m_LogFile{ };

		void LogInternal(LogPriority priority, LogCategory category, std::string const& message) override;


	};
}

#endif