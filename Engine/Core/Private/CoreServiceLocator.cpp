#include "CoreServiceLocator.h"
#include "Logger/NullLogger.h"
namespace MauCor
{
	std::unique_ptr<Logger> CoreServiceLocator::m_pLogger{ std::make_unique<NullLogger>() };

	void CoreServiceLocator::RegisterLogger(std::unique_ptr<Logger>&& pLogger)
	{
		m_pLogger = ((!pLogger) ? std::make_unique<NullLogger>() : std::move(pLogger));
	}
}
