#ifndef MAUCOR_DEFERREDEVENT_H
#define MAUCOR_DEFERREDEVENT_H

#include "../Shared/AssertsInternal.h"
#include "Delegate.h"

namespace MauCor
{
	class IDeferredEvent
	{
	public:
		IDeferredEvent() = default;
		virtual ~IDeferredEvent() = default;

		virtual void Dispatch() = 0;

		IDeferredEvent(IDeferredEvent const&) = delete;
		IDeferredEvent(IDeferredEvent&&) = delete;
		IDeferredEvent& operator=(IDeferredEvent const&) = delete;
		IDeferredEvent& operator=(IDeferredEvent&&) = delete;
	};
}

#endif