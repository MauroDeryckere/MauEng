#include "CorePCH.h"

#include "Logger.h"

namespace MauCor
{
	void Logger::SetPriorityLevel(LogPriority priority) noexcept
	{
		m_LogPriority = priority;
	}
}
