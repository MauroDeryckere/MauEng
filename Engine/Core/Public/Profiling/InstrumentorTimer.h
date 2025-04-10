#ifndef MAUCOR_TIMER_H
#define MAUCOR_TIMER_H

#include <chrono>

namespace MauCor
{
	class InstrumentorTimer final
	{
	public:
		explicit InstrumentorTimer(char const* timerName, bool isFunction);
		~InstrumentorTimer();

		void Stop() noexcept;

		InstrumentorTimer(InstrumentorTimer const&) = default;
		InstrumentorTimer(InstrumentorTimer&&) = default;
		InstrumentorTimer& operator=(InstrumentorTimer const&) = default;
		InstrumentorTimer& operator=(InstrumentorTimer&&) = default;
	private:
		char const* m_Name{ "NO NAME" };

		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartPoint{ };
		bool m_IsStopped{ false };

		bool m_IsFunction{};
	};
}

#endif