#include "Logger/LogCategories.h"


#include "CoreServicelocator.h"

DEFINE_LOG_CATEGORY(LogEngine);
DEFINE_LOG_CATEGORY(LogRenderer);
DEFINE_LOG_CATEGORY(LogCore);
DEFINE_LOG_CATEGORY(LogAudio);
DEFINE_LOG_CATEGORY(LogGame);

void MauCor::LogCategory::SetPriority(MauCor::ELogPriority priority) noexcept
{
	if (priority <= LOG_STRIP_LEVEL)
	{
		ME_LOG_ERROR(LogCore, "Trying to set logger priority level to {}, but it is below the strip level of {}", Logger::PriorityToString(priority), Logger::PriorityToString(LOG_STRIP_LEVEL));
		return;
	}
	m_Priority = priority;
}