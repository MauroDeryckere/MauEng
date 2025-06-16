#ifndef MAUCOR_EVENTMANAGER_H
#define MAUCOR_EVENTMANAGER_H

#include "Singleton.h"
#include <queue>
#include <unordered_map>

namespace MauCor
{
	class IDeferredEvent;
	class IDelegateDelayedUnSubscription;

	class EventManager final : public MauCor::Singleton<EventManager>
	{
	public:
		void ProcessEvents() noexcept;
		void Enqueue(std::unique_ptr<IDeferredEvent>&& event) noexcept;
		void EnqueueUnSub(void const* delegate, std::unique_ptr<IDelegateDelayedUnSubscription>&& unSub) noexcept;
		[[nodiscard]] bool HasUnSubForDelegate(void const* delegate) const noexcept;
		EventManager(EventManager const&) = delete;
		EventManager(EventManager&&) = delete;
		EventManager& operator=(EventManager const&) = delete;
		EventManager& operator=(EventManager&&) = delete;

	private:
		friend class Singleton<EventManager>;
		EventManager() = default;
		virtual ~EventManager() override = default;
		std::queue<std::unique_ptr<IDeferredEvent>> m_EventQueue;

		// Make this a uo set
		std::unordered_map<void const*, std::unique_ptr<IDelegateDelayedUnSubscription>> m_UnSubs;

		void ProcessUnsubscribes() noexcept;
	};
}

#endif