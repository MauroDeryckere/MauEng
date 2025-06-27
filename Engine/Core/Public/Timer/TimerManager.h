#ifndef MAUCOR_TIMER_MANAGER_H
#define MAUCOR_TIMER_MANAGER_H

#include "Events/ListenerHandlers.h"

namespace MauCor
{
	class TimerManager final
	{
	public:
		TimerManager() = default;
		~TimerManager() = default;

		template<typename Callable>
		ListenerHandle const& SetTimer(Callable&& callable, float duration, bool isLooping = false, void* owner = nullptr) noexcept
		{
			m_Timers.emplace_back(duration, duration, isLooping, false, 
				std::make_unique<CallableHandler<void>>(++m_NextTimerId, owner, std::forward<Callable>(callable)) );

			return m_Timers.back().handler->GetHandle();
		}
		//TODO member functions



		TimerManager(TimerManager const&) = delete;
		TimerManager(TimerManager&&) = delete;
		TimerManager& operator=(TimerManager const&) = delete;
		TimerManager& operator=(TimerManager&&) = delete;

	private:
		friend class SceneManager;
		void Tick(float elapsedSec) noexcept
		{
			
		}


		struct Timer final
		{
			float remainingTime;
			float duration;

			bool isLooping;
			bool isPaused;

			std::unique_ptr<IListenerHandler<>> handler;
		};

		std::vector<Timer> m_Timers{};
		uint32_t m_NextTimerId{};
	};
}
#endif
