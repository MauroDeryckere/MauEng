#include "CorePCH.h"

#include "Logger/Logger.h"

namespace MauCor
{
	void Logger::SetPriorityLevel(ELogPriority priority) noexcept
	{
		if (priority <= LOG_STRIP_LEVEL)
		{
			ME_LOG_ERROR(LogCore, "Trying to set logger priority level to {}, but it is below the strip level of {}", PriorityToString(priority), PriorityToString(LOG_STRIP_LEVEL));
			return;
		}

		m_LogPriority = priority;
	}
}
