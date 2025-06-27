#ifndef MAUCOR_TIMER_MANAGER_H
#define MAUCOR_TIMER_MANAGER_H

#include "Events/ListenerHandlers.h"

namespace MauEng
{
	class Scene;
}

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
			m_Timers.emplace_back(duration, duration,
								std::make_unique<CallableHandler<void>>(ListenerHandle{ m_NextTimerId, owner }, std::forward<Callable>(callable)),
								isLooping, false
				);

			m_TimerID_TimerVecIdx.emplace(m_NextTimerId, static_cast<uint32_t>(m_Timers.size() - 1));
			++m_NextTimerId;
			return m_Timers.back().handler->GetHandle();
		}
		template<typename Callable>
		ListenerHandle const& SetTimer(ListenerHandle const& handle, Callable&& callable, float duration = 0.f, bool isLooping = false, void* owner = nullptr) noexcept
		{
			auto const it{ m_TimerID_TimerVecIdx.find(handle.id) };

			// Reset existing timer
			if (it != m_TimerID_TimerVecIdx.end())
			{
				auto& timer = m_Timers[it->second];
				if (duration == 0.f)
				{
					timer.remainingTime = timer.duration;
					timer.isLooping = isLooping;
				}
				else
				{
					timer.remainingTime = duration;
					timer.duration = duration;
					timer.isLooping = isLooping;
				}

				timer.handler = std::make_unique<CallableHandler<void>>(handle, std::forward<Callable>(callable));
				return timer.handler->GetHandle();
			}

			// Add new timer
			return SetTimer(std::forward<Callable>(callable), duration, isLooping, owner);
		}

		void ResetTimer(ListenerHandle const& handle, float newDuration = 0.f, bool isLooping = false) noexcept
		{
			auto const it{ m_TimerID_TimerVecIdx.find(handle.id) };
			if (it == end(m_TimerID_TimerVecIdx))
			{
				ME_LOG_WARN(LogCore, "Trying to reset a timer but timer does not currently exists");
				return;
			}

			auto& timer{ m_Timers[it->second] };
			if (newDuration == 0.f)
			{
				timer.remainingTime = timer.duration;
				timer.isLooping = isLooping;
			}
			else
			{
				timer.remainingTime = newDuration;
				timer.duration = newDuration;
				timer.isLooping = isLooping;
			}
		}

		[[nodiscard]] bool IsTimerActive(ListenerHandle const& handle) const noexcept
		{
			auto const it{ m_TimerID_TimerVecIdx.find(handle.id) };
			if (it == end(m_TimerID_TimerVecIdx))
			{
				ME_LOG_WARN(LogCore, "Trying to reset check if a timer is active, but the timer does not exist");
				return false;
			}

			return not m_Timers[it->second].isPaused and m_Timers[it->second].remainingTime > 0.f and not m_Timers[it->second].pendingRemove;
		}

		[[nodiscard]] bool Exists(ListenerHandle const& handle) const noexcept
		{
			return m_TimerID_TimerVecIdx.contains(handle.id);
		}

		[[nodiscard]] float GetRemainingTime(ListenerHandle const& handle) const noexcept
		{
			auto const it{ m_TimerID_TimerVecIdx.find(handle.id) };
			if (it == end(m_TimerID_TimerVecIdx))
			{
				ME_LOG_WARN(LogCore, "Trying to get timer remaining time, but the timer does not exist");
				return 0.f;
			}

			return m_Timers[it->second].remainingTime;
		}

		bool RemoveTimer(ListenerHandle const& handle) noexcept
		{
			auto const it{ m_TimerID_TimerVecIdx.find(handle.id) };
			if (it == m_TimerID_TimerVecIdx.end())
			{
				ME_LOG_WARN(LogCore, "Trying to remove timer, but the timer does not exist");
				return false;
			}

			m_Timers[it->second].pendingRemove = true;
			return true;
		}

		void RemoveAllTimers(void const* owner) noexcept
		{
			for (auto& timer : m_Timers)
			{
				if (timer.handler->GetHandle().owner == owner)
				{
					timer.pendingRemove = true;
				}
			}
		}

		//TODO member functions
		//template<typename T>
		//ListenerHandle const& SetTimer(T* obj, void (T::* memFn)(), float duration, bool isLooping = false, void* owner = nullptr) noexcept
		//{
		//	ListenerHandle handle{ ++m_NextTimerId, owner };
		//	m_Timers.emplace_back(duration, duration, isLooping, false,
		//		std::make_unique<MemberFunHandler<T, void>>(handle, obj, memFn));
		//	return m_Timers.back().handler->GetHandle();
		//}

		TimerManager(TimerManager const&) = delete;
		TimerManager(TimerManager&&) = delete;
		TimerManager& operator=(TimerManager const&) = delete;
		TimerManager& operator=(TimerManager&&) = delete;

	private:
		friend class MauEng::Scene;
		void Tick() noexcept;

		struct Timer final
		{
			float remainingTime;
			float duration;

			std::unique_ptr<IListenerHandler<>> handler;

			bool isLooping;
			bool isPaused;

			bool pendingRemove{ false };
		};

		std::vector<Timer> m_Timers{};
		std::unordered_map<uint32_t, uint32_t> m_TimerID_TimerVecIdx{};

		uint32_t m_NextTimerId{ 0 };
	};
}
#endif
