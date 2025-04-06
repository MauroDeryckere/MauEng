#include "ServiceLocator.h"

namespace MauEng
{
	std::unique_ptr<MauRen::Renderer> ServiceLocator::m_pRenderer{ std::make_unique<MauRen::NullRenderer>(nullptr) };
	std::unique_ptr<MauRen::DebugRenderer> ServiceLocator::m_pDebugRenderer{ };
}