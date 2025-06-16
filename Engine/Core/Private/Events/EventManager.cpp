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

		ProcessUnsubscribes();
	}

	void EventManager::Enqueue(std::unique_ptr<IDeferredEvent>&& event) noexcept
	{
		m_EventQueue.push(std::move(event));
	}

	void EventManager::EnqueueUnSub(std::unique_ptr<IDelegateDelayedUnSubscription>&& unSub) noexcept
	{
		m_UnSubs.emplace_back(std::move(unSub));
	}

	void EventManager::ProcessUnsubscribes() noexcept
	{
		for (auto&& u : m_UnSubs)
		{
			u->Invoke();
		}

		m_UnSubs.clear();
	}
}


