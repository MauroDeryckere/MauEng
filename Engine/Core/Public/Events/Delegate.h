#ifndef MAUCOR_DELEGATE_H
#define MAUCOR_DELEGATE_H

#include <vector>
#include <memory>
#include <functional>

#include "../Shared/AssertsInternal.h"

//TODO extra safety check to make sure we dont usub from inside an event or this messes up order or is this just implied to be unsafe

namespace MauCor
{
	struct ListenerHandle final
	{
		uint32_t id{ 0 };
		void* owner{ nullptr };

		bool constexpr operator==(ListenerHandle const& other) const noexcept
		{
			return
			id == other.id
			and owner == other.owner;
		}
		bool constexpr operator!=(ListenerHandle const& other) const noexcept
		{
			return !(*this == other);
		}
	};

	template<typename EventType, typename Callable>
	concept EventCallable = std::invocable<Callable, EventType const&>;

	template<typename EventType>
	class Delegate final
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
			auto callable{ [instance, memFunc](EventType const& e)
				{
					(instance->*memFunc)(e);
				} };

			return Subscribe(callable, owner ? owner : instance);
		}

		// Unsubscribe by handle
		// Returns if any listeners were removed
		bool UnSubscribe(ListenerHandle const& handle) noexcept
		{
			return std::erase_if(m_Listeners, [&](auto const& listener)
				{
					return listener->GetHandle() == handle;
				}) == 1;
		}

		//Unsubscribe by owner
		// Owner should not be a nullptr
		bool UnSubscribeAllByOwner(ListenerHandle const& handle) noexcept
		{
			return UnSubscribeAllByOwner(handle.owner);
		}

		// Unsubscribe by owner
		// Owner should not be a nullptr
		bool UnSubscribeAllByOwner(void const* owner) noexcept
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


		// Broadcast immediately
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

	private:
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

		std::vector<std::unique_ptr<IListenerHandler>> m_Listeners;
		uint32_t m_NextListenerId{ 0 };
	};
}

#endif