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

//TODO consider weak ptr for T* or an IsAlive call

namespace MauCor
{
	struct ListenerHandle final
	{
		uint32_t id{ 0 };
		void* owner{ nullptr };

		bool constexpr operator==(ListenerHandle const& other) const noexcept
		{
			return id == other.id and owner == other.owner;
		}
		bool constexpr operator!=(ListenerHandle const& other) const noexcept
		{
			return !(*this == other);
		}
	};

	template<typename EventType, typename Callable>
	concept EventCallable = std::invocable<Callable, EventType const&>;

	template<typename EventType>
	class DelegateInternal final : public std::enable_shared_from_this<DelegateInternal<EventType>>
	{
	public:
		template<typename Callable>
		requires EventCallable<EventType, Callable>
		ListenerHandle const& Subscribe(Callable&& callable, void* owner = nullptr)
		{
			m_Listeners.emplace_back(std::make_unique<CallableHandler>(ListenerHandle{ ++m_NextListenerId, owner }, std::forward<Callable>(callable)));
			return m_Listeners.back()->GetHandle();
		}

		template<typename T>
		ListenerHandle const& Subscribe(void (T::* memFunc)(EventType const&), T* instance, void* owner = nullptr)
		{
			m_Listeners.emplace_back(std::make_unique<MemberFunHandler<T>>(ListenerHandle{ ++m_NextListenerId, owner ? owner : instance }, instance, memFunc));
			return m_Listeners.back()->GetHandle();
		}

		template<typename T>
		ListenerHandle const& Subscribe(void (T::* memFunc)(EventType const&) const, T const* instance, void* owner = nullptr)
		{
			m_Listeners.emplace_back(std::make_unique<MemberFunHandlerConst<T>>(ListenerHandle{ ++m_NextListenerId, owner ? owner : const_cast<void*>(static_cast<void const*>(instance)) }, instance, memFunc));
			return m_Listeners.back()->GetHandle();
		}

		void ProcessAllUnSubs() noexcept
		{
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

		// Unsubscribe by handle (immediate)
		// Returns if any listeners were removed
		bool UnSubscribeImmediate(ListenerHandle const& handle) noexcept
		{
			return std::erase_if(m_Listeners, [&](auto const& listener)
				{
					return listener->GetHandle() == handle;
				}) == 1;
		}

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

	private:
		class DeferredEvent final : public IDeferredEvent
		{
		public:
			using DelegateType = const MauCor::DelegateInternal<EventType>;

			DeferredEvent(std::weak_ptr<DelegateType> delegate, EventType const& event) :
				IDeferredEvent{ },
				m_pDelegate{ delegate },
				m_Event{ event } { }
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
				m_pDelegate{ delegate } {}

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

		class IListenerHandler
		{
		public:
			explicit IListenerHandler(ListenerHandle const& handle) : m_ListenerHandle{ handle } { }
			virtual ~IListenerHandler() = default;

			virtual void Invoke(EventType const&) const noexcept = 0;
			[[nodiscard]] virtual bool IsValid() const noexcept = 0;

			[[nodiscard]] ListenerHandle const& GetHandle() const noexcept { return m_ListenerHandle; }

			IListenerHandler(IListenerHandler const&) = delete;
			IListenerHandler(IListenerHandler&&) = delete;
			IListenerHandler& operator=(IListenerHandler const&) = delete;
			IListenerHandler& operator=(IListenerHandler&&) = delete;
		protected:
			ListenerHandle m_ListenerHandle;
		};

		class CallableHandler final : public IListenerHandler
		{
		public:
			template<typename Callable>
			requires EventCallable<EventType, Callable>
			explicit CallableHandler(ListenerHandle const& handle, Callable&& cb) :
				IListenerHandler{ handle },
				m_Callback{ std::forward<Callable>(cb) } { }

			~CallableHandler() override = default;

			virtual void Invoke(EventType const& e) const noexcept override { m_Callback(e); }
			[[nodiscard]] virtual bool IsValid() const noexcept override { return true; }

			CallableHandler(CallableHandler const&) = delete;
			CallableHandler(CallableHandler&&) = delete;
			CallableHandler& operator=(CallableHandler const&) = delete;
			CallableHandler& operator=(CallableHandler&&) = delete;

		private:
			std::function<void(EventType const&)> m_Callback;
		};

		template<typename T>
		class MemberFunHandler final : public IListenerHandler
		{
		public:
			using MemFnType = void (T::*)(EventType const&);

			explicit MemberFunHandler(ListenerHandle const& handle, T* obj, MemFnType fn) :
				IListenerHandler{ handle },
				m_pObject{ obj },
				m_MemFn{ fn } { }

			~MemberFunHandler() override = default;

			virtual void Invoke(EventType const& e) const noexcept override { (m_pObject->*m_MemFn)(e); }
			[[nodiscard]] virtual bool IsValid() const noexcept override { return m_pObject != nullptr; }

			MemberFunHandler(MemberFunHandler const&) = delete;
			MemberFunHandler(MemberFunHandler&&) = delete;
			MemberFunHandler& operator=(MemberFunHandler const&) = delete;
			MemberFunHandler& operator=(MemberFunHandler&&) = delete;

		private:
			T* m_pObject;
			MemFnType m_MemFn;
		};

		template<typename T>
		class MemberFunHandlerConst final : public IListenerHandler
		{
		public:
			using MemFnType = void (T::*)(EventType const&) const;

			MemberFunHandlerConst(ListenerHandle const& handle, T const* obj, MemFnType fn) :
				IListenerHandler{ handle },
				m_Object{ obj },
				m_MemFn{ fn } { }

			void Invoke(EventType const& e) const noexcept override
			{
				(m_Object->*m_MemFn)(e);
			}

			bool IsValid() const noexcept override { return m_Object != nullptr; }

			MemberFunHandlerConst(MemberFunHandlerConst const&) = delete;
			MemberFunHandlerConst(MemberFunHandlerConst&&) = delete;
			MemberFunHandlerConst& operator=(MemberFunHandlerConst const&) = delete;
			MemberFunHandlerConst& operator=(MemberFunHandlerConst&&) = delete;

		private:
			T const* m_Object;
			MemFnType m_MemFn;
		};

		std::vector<std::unique_ptr<IListenerHandler>> m_Listeners;
		uint32_t m_NextListenerId{ 0 };

		std::vector<void const*> m_OwnerUnSubs;
		std::vector<ListenerHandle> m_HandleUnSubs;
	};

	template<typename EventType>
	class Delegate final
	{
	public:
		Delegate() : m_pDelegate{ std::make_shared<DelegateInternal<EventType>>() } { }
		~Delegate() = default;

		[[nodiscard]] DelegateInternal<EventType>* Get() const noexcept { return m_pDelegate.get(); }

		Delegate(Delegate const&) = default;
		Delegate(Delegate&&) = default;
		Delegate& operator=(Delegate const&) = default;
		Delegate& operator=(Delegate&&) = default;
	private:
		std::shared_ptr<DelegateInternal<EventType>> m_pDelegate;
	};

}

#endif