#include "Timer/TimerManager.h"

#include "CoreServiceLocator.h"

namespace MauCor
{
	void TimerManager::Tick() noexcept
	{
		float const elapsed{ TIME.ElapsedSec() };

		for (auto& timer : m_Timers)
		{
			if (timer.isPaused || timer.pendingRemove)
			{
				continue;
			}

			timer.remainingTime -= elapsed;

			if (timer.remainingTime <= 0.f)
			{
				if (timer.handler->IsValid())
				{
					if (timer.isLooping)
					{
						float const overrun{ -timer.remainingTime };
						uint32_t const fireCount{ static_cast<uint32_t>(overrun / timer.duration) + 1 };

						for (uint32_t i{ 0 }; i < fireCount; ++i)
						{
							timer.handler->Invoke();
						}

						timer.remainingTime += static_cast<float>(fireCount) * timer.duration;
					}
					else
					{
						timer.handler->Invoke();
						timer.pendingRemove = true;
					}
				}
				else
				{
					timer.pendingRemove = true;
				}
			}
		}

		// Remove all timers marked for deletion
		for (size_t i{ 0 }; i < m_Timers.size();)
		{
			if (m_Timers[i].pendingRemove)
			{
				uint32_t id = m_Timers[i].handler->GetHandle().id;
				m_TimerID_TimerVecIdx.erase(id);

				if (i != m_Timers.size() - 1)
				{
					std::swap(m_Timers[i], m_Timers.back());
					m_TimerID_TimerVecIdx[m_Timers[i].handler->GetHandle().id] = static_cast<uint32_t>(i);
				}
				m_Timers.pop_back();
			}
			else
			{
				++i;
			}
		}
	}

}
