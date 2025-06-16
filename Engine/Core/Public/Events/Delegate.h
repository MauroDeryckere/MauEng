#ifndef MAUCOR_DELEGATE_H
#define MAUCOR_DELEGATE_H

#include <vector>
#include <memory>
#include <functional>

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

		bool UnSubscribe(ListenerHandle const& handle) noexcept
		{
			return std::erase_if(m_Listeners, [&](auto const& listener)
				{
					return listener->GetHandle() == handle;
				}) == 1;
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
			IListenerHandler(ListenerHandle const& handle) : m_ListenerHandle{ handle } { }
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
			explicit CallableHandler(ListenerHandle const& handle, Callable&& cb)
			:
			IListenerHandler{ handle },
			m_Callback{ std::forward<Callable>(cb) } { }

			virtual void Invoke(EventType const& e) const noexcept override { m_Callback(e); }
			[[nodiscard]] virtual bool IsValid() const noexcept override { return true; }

			~CallableHandler() override = default;

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