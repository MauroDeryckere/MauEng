#ifndef MAUCOR_DELEGATE_H
#define MAUCOR_DELEGATE_H

#include <vector>
#include <memory>
#include <functional>

namespace MauCor
{
	template<typename EventType>
	class Delegate final
	{
	public:
		template<typename Callable>
		void Subscribe(Callable&& callable)
		{
			m_Listeners.emplace_back(std::make_unique<CallableHandler>(std::forward<Callable>(callable)));
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
			IListenerHandler() = default;
			virtual ~IListenerHandler() = default;

			virtual void Invoke(EventType const&) const noexcept = 0;
			[[nodiscard]] virtual bool IsValid() const noexcept = 0;

			IListenerHandler(IListenerHandler const&) = delete;
			IListenerHandler(IListenerHandler&&) = delete;
			IListenerHandler& operator=(IListenerHandler const&) = delete;
			IListenerHandler& operator=(IListenerHandler&&) = delete;
		};

		class CallableHandler final : public IListenerHandler
		{
		public:
			template<typename Callable>
			explicit CallableHandler(Callable&& cb) : m_Callback(std::forward<Callable>(cb)) {}

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
	};
}

#endif