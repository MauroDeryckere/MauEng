#ifndef MAUCOR_FILELOGGER_H
#define MAUCOR_FILELOGGER_H

#include "Logger/Logger.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace MauCor
{
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

		const uint32_t MAX_FILE_SIZE_BEFORE_ROTATE{ 5'000 };

		void LogInternal(ELogPriority priority, std::string_view const category, std::string_view const message) override;

		void OpenLogFile();
		void RotateFile();
	};
}

#endif