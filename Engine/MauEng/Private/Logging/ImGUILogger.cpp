#include "ImGUILogger.h"

namespace MauEng
{
	void ImGUILogger::LogInternal(MauCor::ELogPriority priority, std::string_view const category, std::string_view const message)
	{
		m_LogBuffer.emplace_back(
			LogMessage{
				.priority = priority,
				.category = std::string{ category },
				.message = std::string{ message }
			});

		if (m_LogBuffer.size() > MAX_LOG_MESSAGES)
		{
			m_LogBuffer.erase(m_LogBuffer.begin(), m_LogBuffer.begin() + (m_LogBuffer.size() - MAX_LOG_MESSAGES));
		}
	}
}
