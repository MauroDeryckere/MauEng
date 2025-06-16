#ifndef MAUCOR_DELEGATEDELAYEDUNSUBSCRIPTION_H
#define MAUCOR_DELEGATEDELAYEDUNSUBSCRIPTION_H

namespace MauCor
{
	class IDelegateDelayedUnSubscription
	{
	public:
		IDelegateDelayedUnSubscription() = default;
		virtual ~IDelegateDelayedUnSubscription() = default;

		virtual void Invoke() = 0;

		IDelegateDelayedUnSubscription(IDelegateDelayedUnSubscription const&) = delete;
		IDelegateDelayedUnSubscription(IDelegateDelayedUnSubscription&&) = delete;
		IDelegateDelayedUnSubscription& operator=(IDelegateDelayedUnSubscription const&) = delete;
		IDelegateDelayedUnSubscription& operator=(IDelegateDelayedUnSubscription&&) = delete;
	};
}

#endif