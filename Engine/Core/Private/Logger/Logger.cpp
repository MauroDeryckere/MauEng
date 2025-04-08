#include "CorePCH.h"

#include "Logger/Logger.h"

namespace MauCor
{
	void Logger::SetPriorityLevel(LogPriority priority) noexcept
	{
		m_LogPriority = priority;
	}
}
