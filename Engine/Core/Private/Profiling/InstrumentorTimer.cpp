#include "Profiling/InstrumentorTimer.h"

#include <iostream>

#include "Profiling/Instrumentor.h"

namespace MauCor
{
	InstrumentorTimer::InstrumentorTimer(char const* timerName, bool isFunction):
		m_Name{ timerName },
		m_IsFunction{isFunction}
	{
		m_StartPoint = std::chrono::high_resolution_clock::now();
	}

	InstrumentorTimer::~InstrumentorTimer()
	{
		if (!m_IsStopped)
		{
			Stop();
		}
	}

	void InstrumentorTimer::Stop() noexcept
	{
		auto const endPoint{ std::chrono::high_resolution_clock::now() };
		auto const start{ std::chrono::time_point_cast<std::chrono::microseconds>(m_StartPoint).time_since_epoch().count() };
		auto const end{ std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count() };

		m_IsStopped = true;

		uint32_t const threadID{ static_cast<uint32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id())) };
		Instrumentor::GetInstance().WriteProfile({ m_Name, start, end, threadID }, m_IsFunction);
	}
}
