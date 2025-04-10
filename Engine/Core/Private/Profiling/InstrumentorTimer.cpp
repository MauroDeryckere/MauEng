#include "Profiling/InstrumentorTimer.h"

#include <iostream>

#include "Profiling/Instrumentor.h"

namespace MauCor
{
	//thread_local uint32_t InstrumentorTimer::m_NestCount = 0;

	InstrumentorTimer::InstrumentorTimer(char const* timerName, bool isFunction):
		m_Name{ timerName },
		m_IsFunction{isFunction}
	{
		m_StartPoint = std::chrono::high_resolution_clock::now();
		//++m_NestCount;
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
		auto end{ std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count() };

		m_IsStopped = true;

		Instrumentor::GetInstance().WriteProfile({ m_Name, start, end, std::this_thread::get_id() }, m_IsFunction);
		//--m_NestCount;
	}
}
