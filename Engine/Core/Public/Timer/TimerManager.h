#ifndef MAUCOR_TIMER_MANAGER_H
#define MAUCOR_TIMER_MANAGER_H

#include "Events/ListenerHandlers.h"
#include "../Shared/AssertsInternal.h"
// Timers do require manual cleanup (RemoveTimer) when they are no longer needed, otherwise they will linger in memory and may cause invalid access errors.

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
			ME_CORE_ASSERT(duration >= 0.f);

			m_Timers.emplace_back(duration, duration,
								std::make_unique<CallableHandler<void>>(ListenerHandle{ m_NextTimerId, owner }, std::forward<Callable>(callable)),
								isLooping, false
				);

			m_TimerID_TimerVecIdx.emplace(m_NextTimerId, static_cast<uint32_t>(m_Timers.size() - 1));
			++m_NextTimerId;
			return m_Timers.back().handler->GetHandle();
		}
		template<typename Callable>
		void SetTimerForNextTick(Callable&& callable) noexcept
		{
			m_NextTickHandlers.emplace_back(std::make_unique<CallableHandler<void>>(ListenerHandle{}, std::forward<Callable>(callable)));
		}
		template<typename Callable>
		ListenerHandle const& SetTimer(ListenerHandle const& handle, Callable&& callable, float duration = 0.f, bool isLooping = false, void* owner = nullptr) noexcept
		{
			ME_CORE_ASSERT(duration >= 0.f);

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

		template<typename T>
		ListenerHandle const& SetTimer(void (T::* memFunc)(), T* instance, float duration, bool isLooping = false, void* owner = nullptr) noexcept
		{
			ME_CORE_ASSERT(duration >= 0.f);

			m_Timers.emplace_back(duration, duration,
				std::make_unique<MemberFunHandler<T, void>>(ListenerHandle{ m_NextTimerId, (owner ? owner : instance) }, instance, memFunc),
				isLooping, false
			);

			++m_NextTimerId;
			return m_Timers.back().handler->GetHandle();
		}
		template<typename T>
		void SetTimerForNextTick(void (T::* memFunc)(), T* instance) noexcept
		{
			m_NextTickHandlers.emplace_back(std::make_unique<MemberFunHandler<T, void>>(ListenerHandle{}, instance, memFunc));
		}
		template<typename T>
		ListenerHandle const& SetTimer(ListenerHandle const& handle, void (T::* memFunc)(), T* instance, float duration, bool isLooping = false, void* owner = nullptr) noexcept
		{
			ME_CORE_ASSERT(duration >= 0.f);
			auto const it{ m_TimerID_TimerVecIdx.find(handle.id) };

			// Reset existing timer
			if (it != m_TimerID_TimerVecIdx.end())
			{
				auto& timer{ m_Timers[it->second] };
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

				timer.handler = std::make_unique<MemberFunHandler<T, void>>(handle, instance, memFunc);
				return timer.handler->GetHandle();
			}

			return SetTimer(memFunc, instance, duration, isLooping, (owner ? owner : instance));
		}

		template<typename T>
		ListenerHandle const& SetTimer(void (T::* memFunc)() const, T const* instance, float duration, bool isLooping = false, void* owner = nullptr) noexcept
		{
			ME_CORE_ASSERT(duration >= 0.f);

			m_Timers.emplace_back(duration, duration,
				std::make_unique<MemberFunHandlerConst<T, void>>(ListenerHandle{ m_NextTimerId, (owner ? owner : const_cast<void*>(static_cast<void const*>(instance))) }, instance, memFunc),
				isLooping, false
			);

			++m_NextTimerId;
			return m_Timers.back().handler->GetHandle();
		}
		template<typename T>
		void SetTimerForNextTick(void (T::* memFunc)() const, T const* instance) noexcept
		{
			m_NextTickHandlers.emplace_back(std::make_unique<MemberFunHandlerConst<T, void>>(ListenerHandle{}, instance, memFunc));
		}
		template<typename T>
		ListenerHandle const& SetTimer(ListenerHandle const& handle, void (T::* memFunc)() const, T const* instance, float duration, bool isLooping = false, void* owner = nullptr) noexcept
		{
			ME_CORE_ASSERT(duration >= 0.f);
			auto const it{ m_TimerID_TimerVecIdx.find(handle.id) };

			// Reset existing timer
			if (it != m_TimerID_TimerVecIdx.end())
			{
				auto& timer{ m_Timers[it->second] };
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

				timer.handler = std::make_unique<MemberFunHandlerConst<T, void>>(handle, instance, memFunc);
				return timer.handler->GetHandle();
			}

			return SetTimer(memFunc, instance, duration, isLooping, (owner ? owner : const_cast<void*>(static_cast<void const*>(instance))));
		}

		void ResetTimer(ListenerHandle const& handle, float newDuration = 0.f, bool isLooping = false) noexcept
		{
			ME_CORE_ASSERT(newDuration >= 0.f);

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

		[[nodiscard]] bool IsTimerPaused(ListenerHandle const& handle) const noexcept
		{
			auto const it{ m_TimerID_TimerVecIdx.find(handle.id) };

			if (it == end(m_TimerID_TimerVecIdx))
			{
				ME_LOG_WARN(LogCore, "Trying to check if timer is paused, but the timer does not exist");
				return false;
			}

			return m_Timers[it->second].isPaused;
		}

		bool PauseTimer(ListenerHandle const& handle) noexcept
		{
			auto const it{ m_TimerID_TimerVecIdx.find(handle.id) };
			if (it == end(m_TimerID_TimerVecIdx))
			{
				ME_LOG_WARN(LogCore, "Trying to pause timer, but the timer does not exist");
				return false;
			}

			m_Timers[it->second].isPaused = true;
			return true;
		}

		bool ResumeTimer(ListenerHandle const& handle) noexcept
		{
			auto const it{ m_TimerID_TimerVecIdx.find(handle.id) };
			if (it == end(m_TimerID_TimerVecIdx))
			{
				ME_LOG_WARN(LogCore, "Trying to resume timer, but the timer does not exist");
				return false;
			}

			m_Timers[it->second].isPaused = false;
			return true;
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

		void PauseAllTimers(void const* owner) noexcept
		{
			ME_CORE_ASSERT(owner);

			for (auto& timer : m_Timers)
			{
				if (timer.handler->GetHandle().owner == owner)
				{
					timer.isPaused = true;
				}
			}
		}

		void RemoveAllTimers(void const* owner) noexcept
		{
			ME_CORE_ASSERT(owner);

			for (auto& timer : m_Timers)
			{
				if (timer.handler->GetHandle().owner == owner)
				{
					timer.pendingRemove = true;
				}
			}
		}

		[[nodiscard]] bool IsTimerExpired(ListenerHandle const& handle) const noexcept
		{
			auto const it{ m_TimerID_TimerVecIdx.find(handle.id) };
			if (it == end(m_TimerID_TimerVecIdx))
			{
				ME_LOG_WARN(LogCore, "Trying to check if timer is expired, but the timer does not exist");
				return false;
			}
			
			return m_Timers[it->second].pendingRemove;
		}

		void Clear() noexcept
		{
			m_Timers.clear();
			m_TimerID_TimerVecIdx.clear();
			m_NextTickHandlers.clear();
		}

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

		std::vector<std::unique_ptr<IListenerHandler<>>> m_NextTickHandlers;

		uint32_t m_NextTimerId{ 0 };
	};
}
#endif
