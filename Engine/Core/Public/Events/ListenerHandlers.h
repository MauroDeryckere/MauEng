#ifndef MAUCOR_LISTENERHANDLERS_H
#define MAUCOR_LISTENERHANDLERS_H

#include "ListenerHandle.h"

namespace MauCor
{
	template<typename ParamType, typename Callable>
	concept CallableWithParam = std::invocable<Callable, ParamType const&>;

	template<typename ParamType>
	class IListenerHandler
	{
	public:
		explicit IListenerHandler(ListenerHandle const& handle) : m_ListenerHandle{ handle } {}
		virtual ~IListenerHandler() = default;

		virtual void Invoke(ParamType const&) const noexcept = 0;
		[[nodiscard]] virtual bool IsValid() const noexcept = 0;

		[[nodiscard]] ListenerHandle const& GetHandle() const noexcept { return m_ListenerHandle; }

		IListenerHandler(IListenerHandler const&) = delete;
		IListenerHandler(IListenerHandler&&) = delete;
		IListenerHandler& operator=(IListenerHandler const&) = delete;
		IListenerHandler& operator=(IListenerHandler&&) = delete;
	protected:
		ListenerHandle m_ListenerHandle;
	};

	template<typename ParamType>
	class CallableHandler final : public IListenerHandler<ParamType>
	{
	public:
		template<typename Callable>
			requires CallableWithParam<ParamType, Callable>
		explicit CallableHandler(ListenerHandle const& handle, Callable&& cb) :
			IListenerHandler<ParamType>{ handle },
			m_Callback{ std::forward<Callable>(cb) } {
		}

		~CallableHandler() override = default;

		virtual void Invoke(ParamType const& e) const noexcept override { m_Callback(e); }
		[[nodiscard]] virtual bool IsValid() const noexcept override { return true; }

		CallableHandler(CallableHandler const&) = delete;
		CallableHandler(CallableHandler&&) = delete;
		CallableHandler& operator=(CallableHandler const&) = delete;
		CallableHandler& operator=(CallableHandler&&) = delete;

	private:
		std::function<void(ParamType const&)> m_Callback;
	};

	template<typename T, typename ParamType>
	class MemberFunHandler final : public IListenerHandler<ParamType>
	{
	public:
		using MemFnType = void (T::*)(ParamType const&);

		explicit MemberFunHandler(ListenerHandle const& handle, T* obj, MemFnType fn) :
			IListenerHandler<ParamType>{ handle },
			m_pObject{ obj },
			m_MemFn{ fn } {
		}

		~MemberFunHandler() override = default;

		virtual void Invoke(ParamType const& e) const noexcept override { (m_pObject->*m_MemFn)(e); }
		[[nodiscard]] virtual bool IsValid() const noexcept override { return m_pObject != nullptr; }

		MemberFunHandler(MemberFunHandler const&) = delete;
		MemberFunHandler(MemberFunHandler&&) = delete;
		MemberFunHandler& operator=(MemberFunHandler const&) = delete;
		MemberFunHandler& operator=(MemberFunHandler&&) = delete;

	private:
		T* m_pObject;
		MemFnType m_MemFn;
	};

	template<typename T, typename ParamType>
	class MemberFunHandlerConst final : public IListenerHandler<ParamType>
	{
	public:
		using MemFnType = void (T::*)(ParamType const&) const;

		MemberFunHandlerConst(ListenerHandle const& handle, T const* obj, MemFnType fn) :
			IListenerHandler<ParamType>{ handle },
			m_Object{ obj },
			m_MemFn{ fn } {
		}

		void Invoke(ParamType const& e) const noexcept override
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
}

#endif