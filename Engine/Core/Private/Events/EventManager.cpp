#include "Events/EventManager.h"

#include "Events/DeferredEvent.h"

namespace MauCor
{
	void EventManager::ProcessEvents() noexcept
	{
		ProcessUnsubscribes();

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

	void EventManager::EnqueueUnSub(void const* delegate, std::unique_ptr<IDelegateDelayedUnSubscription>&& unSub) noexcept
	{
		m_UnSubs[delegate] = std::move(unSub);
	}

	bool EventManager::HasUnSubForDelegate(void const* delegate) const noexcept
	{
		auto it{ m_UnSubs.find(delegate) };
		return it != end(m_UnSubs);
	}

	void EventManager::ProcessUnsubscribes() noexcept
	{
		for (auto&& u : m_UnSubs)
		{
			u.second->Invoke();
		}

		m_UnSubs.clear();
	}
}


