#include "CorePCH.h"

#include "Logger/Logger.h"

namespace MauCor
{
	void Logger::SetPriorityLevel(ELogPriority priority) noexcept
	{
		m_LogPriority = priority;
	}
}
