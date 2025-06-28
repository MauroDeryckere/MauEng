#ifndef MAUCOR_DELEGATE_H
#define MAUCOR_DELEGATE_H

#include <vector>
#include <memory>
#include <functional>

#include "DeferredEvent.h"
#include "Delegate.h"
#include "EventManager.h"
#include "DelegateDelayedUnSubscription.h"
#include "../Shared/AssertsInternal.h"

#include "ListenerHandlers.h"

// TODO void specialization for no param delegates (fine for now, just use an empty struct)

// TODO consider weak ptr for T* or an IsAlive call in member function handler
// Because this could be an issue when it's an end of frame event, and the object has been destroyed when the event fires (object destroys should be delayed until after anyway though)
// But if object that isn't managed by engine (?)

// Delegate is in class A
// class B is subscribed & is deleted
// Scenario1: did not unsub, event fires -> PROBLEM
// Scenario2: did unsub but Queued broadcast -> PROBLEM

// Fix would be some form of a dynamic/ management system for this

namespace MauCor
{
	template<typename EventType>
	class DelegateInternal final : public std::enable_shared_from_this<DelegateInternal<EventType>>
	{
	public:
		template<typename Callable>
			requires CallableWithParam<EventType, Callable>
		ListenerHandle const& Subscribe(Callable&& callable, void* owner = nullptr)
		{
			m_Listeners.emplace_back(std::make_unique<CallableHandler<EventType>>(ListenerHandle{ ++m_NextListenerId, owner }, std::forward<Callable>(callable)));
			return m_Listeners.back()->GetHandle();
		}

		template<typename T>
		ListenerHandle const& Subscribe(void (T::* memFunc)(EventType const&), T* instance, void* owner = nullptr)
		{
			m_Listeners.emplace_back(std::make_unique<MemberFunHandler<T, EventType>>(ListenerHandle{ ++m_NextListenerId, owner ? owner : instance }, instance, memFunc));
			return m_Listeners.back()->GetHandle();
		}

		template<typename T>
		ListenerHandle const& Subscribe(void (T::* memFunc)(EventType const&) const, T const* instance, void* owner = nullptr)
		{
			m_Listeners.emplace_back(std::make_unique<MemberFunHandlerConst<T, EventType>>(ListenerHandle{ ++m_NextListenerId, owner ? owner : const_cast<void*>(static_cast<void const*>(instance)) }, instance, memFunc));
			return m_Listeners.back()->GetHandle();
		}

		void ProcessAllUnSubs() noexcept
		{
			if (m_ShouldClear)
			{
				m_OwnerUnSubs.clear();
				m_HandleUnSubs.clear();
				m_Listeners.clear();

				m_ShouldClear = false;
			}

			for (auto& u : m_OwnerUnSubs)
			{
				UnSubscribeAllByOwnerImmediate(u);
			}
			for (auto& u : m_HandleUnSubs)
			{
				UnSubscribeImmediate(u);
			}

			m_OwnerUnSubs.clear();
			m_HandleUnSubs.clear();
		}

		// Unsubscribe by handle (delayed)
		void UnSubscribe(ListenerHandle const& handle) noexcept
		{
			auto& e{ EventManager::GetInstance() };

			m_HandleUnSubs.emplace_back(handle);
			if (not e.HasUnSubForDelegate(this))
			{
				auto self{ this->weak_from_this() };
				e.EnqueueUnSub(this, std::make_unique<DelegateDelayedUnSub>(self));
			}
		}

		// Unsubscribe by owner (delayed)
		void UnSubscribe(void const* owner) noexcept
		{
			UnSubscribeAllByOwner(owner);
		}

		// Unsubscribe by handle (immediate)
		// Returns if any listeners were removed
		bool UnSubscribeImmediate(ListenerHandle const& handle) noexcept
		{
			return std::erase_if(m_Listeners, [&](auto const& listener)
				{
					return listener->GetHandle() == handle;
				}) == 1;
		}

		// Unsubscribe by owner (immediate)
		// Returns if any listeners were removed
		bool UnSubscribeImmediate(void const* owner) noexcept
		{
			return UnSubscribeAllByOwnerImmediate(owner);
		}

		// Unsubscribe by owner (delayed)
		void UnSubscribeAllByOwner(ListenerHandle const& handle) noexcept
		{
			UnSubscribeAllByOwner(handle.owner);
		}

		// Unsubscribe by owner (immediate)
		// Owner should not be a nullptr
		bool UnSubscribeAllByOwnerImmediate(ListenerHandle const& handle) noexcept
		{
			return UnSubscribeAllByOwnerImmediate(handle.owner);
		}

		void UnSubscribeAllByOwner(void const* owner) noexcept
		{
			ME_CORE_ASSERT(nullptr != owner, "Trying to unsubscribe all listeners by owner but owner is null");
			if (!owner)
			{
				return;
			}

			auto& e{ EventManager::GetInstance() };
			m_OwnerUnSubs.emplace_back(owner);

			if (not e.HasUnSubForDelegate(this))
			{
				auto self{ this->weak_from_this() };
				e.EnqueueUnSub(this, std::make_unique<DelegateDelayedUnSub>(self));
			}
		}

		// Unsubscribe by owner (immediate)
		// Owner should not be a nullptr
		bool UnSubscribeAllByOwnerImmediate(void const* owner) noexcept
		{
			ME_CORE_ASSERT(nullptr != owner, "Trying to unsubscribe all listeners by owner but owner is null");
			
			if (!owner)
			{
				return false;
			}

			return std::erase_if(m_Listeners, [&](auto const& listener)
				{
					return listener->GetHandle().owner == owner;
				}) > 0;
		}


		// Broadcast immediately (blocking broadcast)
		void Broadcast(EventType const& event) const noexcept
		{
			for (auto&& l : m_Listeners)
			{
				if (l->IsValid())
				{
					l->Invoke(event);
				}
			}
		}

		//Broadcast event for end of the frame (non blocking broadcast)
		void QueueBroadcast(EventType const& event) const noexcept
		{
			auto& e{ EventManager::GetInstance() };

			auto self{ this->weak_from_this() };
			e.Enqueue(std::make_unique<DeferredEvent>(self, event));
		}

		void Clear() noexcept
		{
			m_ShouldClear = true;
			auto& e{ EventManager::GetInstance() };

			if (not e.HasUnSubForDelegate(this))
			{
				auto self{ this->weak_from_this() };
				e.EnqueueUnSub(this, std::make_unique<DelegateDelayedUnSub>(self));
			}
		}

	private:
		std::vector<std::unique_ptr<IListenerHandler<EventType>>> m_Listeners;
		uint32_t m_NextListenerId{ 0 };

		std::vector<void const*> m_OwnerUnSubs;
		std::vector<ListenerHandle> m_HandleUnSubs;

		bool m_ShouldClear { false };

#pragma region PrivateTemplatedClasses
		class DeferredEvent final : public IDeferredEvent
		{
		public:
			using DelegateType = const MauCor::DelegateInternal<EventType>;

			DeferredEvent(std::weak_ptr<DelegateType> delegate, EventType const& event) :
				IDeferredEvent{ },
				m_pDelegate{ delegate },
				m_Event{ event } {
			}
			virtual ~DeferredEvent() override = default;

			virtual void Dispatch() override
			{
				if (auto ptr{ m_pDelegate.lock() })
				{
					ptr->Broadcast(m_Event);
				}
			}

			DeferredEvent(DeferredEvent const&) = delete;
			DeferredEvent(DeferredEvent&&) = delete;
			DeferredEvent& operator=(DeferredEvent const&) = delete;
			DeferredEvent& operator=(DeferredEvent&&) = delete;

		private:
			std::weak_ptr<DelegateType> m_pDelegate;
			EventType m_Event;
		};

		class DelegateDelayedUnSub final : public IDelegateDelayedUnSubscription
		{
		public:
			using DelegateType = MauCor::DelegateInternal<EventType>;

			explicit DelegateDelayedUnSub(std::weak_ptr<DelegateType> delegate) :
				IDelegateDelayedUnSubscription{ },
				m_pDelegate{ delegate } {
			}

			virtual ~DelegateDelayedUnSub() override = default;

			void Invoke() override
			{
				if (auto ptr{ m_pDelegate.lock() })
				{
					ptr->ProcessAllUnSubs();
				}
			}

			DelegateDelayedUnSub(DelegateDelayedUnSub const&) = delete;
			DelegateDelayedUnSub(DelegateDelayedUnSub&&) = delete;
			DelegateDelayedUnSub& operator=(DelegateDelayedUnSub const&) = delete;
			DelegateDelayedUnSub& operator=(DelegateDelayedUnSub&&) = delete;

		private:
			std::weak_ptr<DelegateType> m_pDelegate;
		};
#pragma endregion
	};

	template<typename EventType>
	class Delegate final
	{
	public:
		Delegate() : m_pDelegate{ std::make_shared<DelegateInternal<EventType>>() } { }
		~Delegate() = default;

		[[nodiscard]] DelegateInternal<EventType>* Get() const noexcept { return m_pDelegate.get(); }

		// Subscribes
		//Subscribe using const member function
		template<typename T>
		ListenerHandle const& Subscribe(BindingConstMemFn<T, EventType> const& binding) noexcept
		{
			return Get()->Subscribe(binding.memFn, binding.instance, binding.owner);
		}
		template<typename T>
		ListenerHandle const& Subscribe(BindingMemFn<T, EventType> const& binding) noexcept
		{
			return Get()->Subscribe(binding.memFn, binding.instance, binding.owner);
		}
		template<typename Callable>
			requires CallableWithParam<EventType, Callable>
		ListenerHandle const& Subscribe(BindingCallable<Callable> const& binding) noexcept
		{
			return Get()->Subscribe(std::move(binding.callable), binding.owner);
		}

		template<typename T>
		ListenerHandle const& operator+=(BindingConstMemFn<T, EventType> const& binding) noexcept
		{
			return Get()->Subscribe(binding.memFn, binding.instance, binding.owner);
		}
		//Subscribe using member function
		template<typename T>
		ListenerHandle const& operator+=(BindingMemFn<T, EventType> const& binding) noexcept
		{
			return Get()->Subscribe(binding.memFn, binding.instance, binding.owner);
		}
		//Subscribe using callable 
		template<typename Callable>
			requires CallableWithParam<EventType, Callable>
		ListenerHandle const& operator+=(BindingCallable<Callable> const& binding) noexcept
		{
			return Get()->Subscribe(std::move(binding.callable), binding.owner);
		}

		// UnSubs (delayed)
		bool UnSubscribe(void const* owner) noexcept
		{
			return Get()->UnSubscribe(owner);
		}
		bool UnSubscribe(ListenerHandle const& handle) noexcept
		{
			return Get()->UnSubscribe(handle);
		}
		bool UnSubscribeAllByOwner(void const* owner) noexcept
		{
			return Get()->UnSubscribeAllByOwner(owner);
		}
		bool UnSubscribeAllByOwner(ListenerHandle const& handle) noexcept
		{
			return Get()->UnSubscribeAllByOwner(handle);
		}
		// Unsubcribe (delayed)
		void operator-=(void const* owner) noexcept
		{
			Get()->UnSubscribe(owner);
		}
		// Unsubcribe (delayed)
		void operator-=(ListenerHandle const& handle) noexcept
		{
			Get()->UnSubscribe(handle);
		}

		// UnSubs (immediate)
		bool UnSubscribeImmediate(void const* owner) noexcept
		{
			return Get()->UnSubscribeImmediate(owner);
		}
		bool UnSubscribeImmediate(ListenerHandle const& handle) noexcept
		{
			return Get()->UnSubscribeImmediate(handle);
		}
		bool UnSubscribeAllByOwnerImmediate(void const* owner) noexcept
		{
			return Get()->UnSubscribeAllByOwnerImmediate(owner);
		}
		bool UnSubscribeAllByOwnerImmediate(ListenerHandle const& handle) noexcept
		{
			return Get()->UnSubscribeAllByOwnerImmediate(handle);
		}
		// Unsubscribe (immediate)
		bool operator/=(void const* owner) noexcept
		{
			return Get()->UnSubscribeImmediate(owner);
		}
		// Unsubscribe (immediate)
		bool operator/=(ListenerHandle const& handle) noexcept
		{
			return Get()->UnSubscribeImmediate(handle);
		}

		// Broadcasts immediately
		void Broadcast(EventType const& event) const noexcept
		{
			Get()->Broadcast(event);
		}
		// Queue broadcast for end of frame
		void QueueBroadcast(EventType const& event) const noexcept
		{
			Get()->QueueBroadcast(event);
		}

		// Broadcasts immediately
		void operator<(EventType const& event) noexcept
		{
			Broadcast(event);
		}
		// queue Broadcast
		void operator<<(EventType const& event) noexcept
		{
			QueueBroadcast(event);
		}

		void Clear() noexcept
		{
			Get()->Clear();
		}

		Delegate(Delegate const&) = default;
		Delegate(Delegate&&) = default;
		Delegate& operator=(Delegate const&) = default;
		Delegate& operator=(Delegate&&) = default;
	private:
		std::shared_ptr<DelegateInternal<EventType>> m_pDelegate;
	};
}

#endif