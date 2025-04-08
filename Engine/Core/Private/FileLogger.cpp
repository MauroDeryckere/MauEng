#include "CorePCH.h"

#include "FileLogger.h"

namespace MauCor
{
	FileLogger::FileLogger(std::filesystem::path const& path) :
		m_LogFile{ path }
	{

	}

	void FileLogger::LogInternal(LogPriority priority, LogCategory category, std::string const& message)
	{
		//TODO log to file here
	}
}
