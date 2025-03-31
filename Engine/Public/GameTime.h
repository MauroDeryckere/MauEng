#ifndef MAUENG_GAMETIME_H
#define MAUENG_GAMETIME_H

#include "Singleton.h"

#include <chrono>

namespace MauEng
{
	class Time final : public MauCor::Singleton<Time>
	{
	public:
		// Get this frames elapsed time in seconds
		[[nodiscard]] inline float constexpr ElapsedSec() const noexcept { return m_ElapsedSec; }
		// Get the time a fixed update tick takes
		[[nodiscard]] inline float constexpr FixedTimeStepSec() const noexcept { return (m_MsFixedTimeStep / 1000.f); }


		Time(Time const&) = delete;
		Time(Time&&) = delete;
		Time& operator=(Time const&) = delete;
		Time& operator=(Time&&) = delete;
	private:
		friend class Singleton<Time>;
		Time() = default;
		virtual ~Time() override = default;

		// Allow engine to call update functions, but users can not
		friend class Engine;

		std::chrono::steady_clock::time_point m_LastTime{ std::chrono::high_resolution_clock::now() };

		// Ms for a single frame -> 1s / MsPerFrame in seconds == FPS
		float const m_MsPerFrame{ 16.7f };
		// Ms for a single fixed timestep tick
		float const m_MsFixedTimeStep{ 20.f };

		float m_ElapsedSec{ 0.f };
		float m_MsLag{ 0.f };

#pragma region Engine
		// Process a single frame of fixed timestep lagg
		inline void constexpr ProcessLag() noexcept { m_MsLag -= m_MsFixedTimeStep; }

		// Is there any lag currently we have to catch up with?
		[[nodiscard]] inline bool constexpr IsLag() const noexcept { return m_MsLag >= m_MsFixedTimeStep; }

		// Update the time (elapsed seconds & lag)
		inline void Update() noexcept
		{
			auto const currentTime{ std::chrono::high_resolution_clock::now() };
			m_ElapsedSec = std::chrono::duration<float>(currentTime - m_LastTime).count();

			m_MsLag += m_ElapsedSec * 1000.f;
			m_LastTime = std::chrono::high_resolution_clock::now();
		}

		// How long should we sleep to achieve our Ms per frame target
		[[nodiscard]] inline auto SleepTime() const noexcept
		{
			return m_LastTime + std::chrono::milliseconds(static_cast<long>(m_MsPerFrame)) - std::chrono::high_resolution_clock::now();
		}
#pragma endregion
	};
}

#endif