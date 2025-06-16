#ifndef MAUCOR_EVENTMANAGER_H
#define MAUCOR_EVENTMANAGER_H

#include "Singleton.h"
#include <queue>

namespace MauCor
{
	class IDeferredEvent;

	class EventManager final : public MauCor::Singleton<EventManager>
	{
	public:
		void ProcessEvents() noexcept;
		void Enqueue(std::unique_ptr<IDeferredEvent>&& event) noexcept;

		void ProcessUnsubscribes() noexcept;

		EventManager(EventManager const&) = delete;
		EventManager(EventManager&&) = delete;
		EventManager& operator=(EventManager const&) = delete;
		EventManager& operator=(EventManager&&) = delete;

	private:
		friend class Singleton<EventManager>;
		EventManager() = default;
		virtual ~EventManager() override = default;
		std::queue<std::unique_ptr<IDeferredEvent>> m_EventQueue;
	};
}

#endif