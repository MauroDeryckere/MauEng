#include "CorePCH.h"

#include "FileLogger.h"

namespace MauCor
{
	FileLogger::FileLogger(std::filesystem::path&& path) :
		m_LogFilePath{ std::move(path) }
	{
		m_LogFile.open(m_LogFilePath, std::ios::out | std::ios::app);
		if (!m_LogFile.is_open())
		{
			std::cerr << "Error opening log file: " << m_LogFilePath.string() << std::endl;
		}
	}

	FileLogger::~FileLogger()
	{
		if (m_LogFile.is_open())
		{
			m_LogFile.close();
		}
	}

	void FileLogger::LogInternal(LogPriority priority, LogCategory category, std::string const& message)
	{
		if (m_LogFile.is_open())
		{
			auto const now{ std::chrono::system_clock::now() };
			std::string const timestamp{ std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), now }) };

			m_LogFile << fmt::format("[{}] [{}] [{}] {}\n", timestamp, PriorityToString(priority), CategoryToString(category), message);
		}
	}
}
