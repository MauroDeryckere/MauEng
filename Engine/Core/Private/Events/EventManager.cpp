#include "Events/EventManager.h"

#include "Events/DeferredEvent.h"

namespace MauCor
{
	void EventManager::ProcessEvents() noexcept
	{
		while (!m_EventQueue.empty())
		{
			m_EventQueue.front()->Dispatch();
			m_EventQueue.pop();
		}
	}

	void EventManager::Enqueue(std::unique_ptr<IDeferredEvent>&& event) noexcept
	{
		m_EventQueue.push(std::move(event));
	}

	void EventManager::ProcessUnsubscribes() noexcept
	{

	}
}


