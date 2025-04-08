#include "CoreServiceLocator.h"

namespace MauCor
{
	std::unique_ptr<Logger> CoreServiceLocator::m_pLogger{ std::make_unique<NullLogger>() };
}