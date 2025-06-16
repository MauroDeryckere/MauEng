#include "Events/EventManager.h"

#include "Events/DeferredEvent.h"

namespace MauCor
{
	void EventManager::ProcessEvents() noexcept
	{
		for (auto&& e : m_EventQueue)
		{
			e->Dispatch();
		}

		m_EventQueue.clear();
	}

	void EventManager::Enqueue(std::unique_ptr<IDeferredEvent>&& event) noexcept
	{
		m_EventQueue.emplace_back(std::move(event));
	}
}


