#ifndef MAUCOR_LISTENERHANDLERS_H
#define MAUCOR_LISTENERHANDLERS_H

#include "ListenerHandle.h"

namespace MauCor
{
	template<typename ParamType, typename Callable>
	concept CallableWithParam = std::invocable<Callable, ParamType const&>;

	template<typename Callable>
	concept CallableNoParam = std::invocable<Callable>;

	template<typename ParamType = void>
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
	template<>
	class IListenerHandler<void>
	{
	public:
		explicit IListenerHandler(ListenerHandle const& handle) : m_ListenerHandle{ handle } {}
		virtual ~IListenerHandler() = default;

		virtual void Invoke() const noexcept = 0;
		[[nodiscard]] virtual bool IsValid() const noexcept = 0;

		[[nodiscard]] ListenerHandle const& GetHandle() const noexcept { return m_ListenerHandle; }

		IListenerHandler(IListenerHandler const&) = delete;
		IListenerHandler(IListenerHandler&&) = delete;
		IListenerHandler& operator=(IListenerHandler const&) = delete;
		IListenerHandler& operator=(IListenerHandler&&) = delete;
	protected:
		ListenerHandle m_ListenerHandle;
	};

	template<typename ParamType = void>
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
	template<>
	class CallableHandler<void> final : public IListenerHandler<void>
	{
	public:
		template<typename Callable>
		explicit CallableHandler(ListenerHandle const& handle, Callable&& cb) :
			IListenerHandler{ handle },
			m_Callback{ std::forward<Callable>(cb) } {
		}

		~CallableHandler() override = default;

		virtual void Invoke() const noexcept override { m_Callback(); }
		[[nodiscard]] virtual bool IsValid() const noexcept override { return true; }

		CallableHandler(CallableHandler const&) = delete;
		CallableHandler(CallableHandler&&) = delete;
		CallableHandler& operator=(CallableHandler const&) = delete;
		CallableHandler& operator=(CallableHandler&&) = delete;

	private:
		std::function<void()> m_Callback;
	};

	template<typename T, typename ParamType = void>
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
	template<typename T>
	class MemberFunHandler<T, void> final : public IListenerHandler<void>
	{
	public:
		using MemFnType = void (T::*)();

		explicit MemberFunHandler(ListenerHandle const& handle, T* obj, MemFnType fn) :
			IListenerHandler{ handle },
			m_pObject{ obj },
			m_MemFn{ fn } {
		}

		~MemberFunHandler() override = default;

		virtual void Invoke() const noexcept override { (m_pObject->*m_MemFn)(); }
		[[nodiscard]] virtual bool IsValid() const noexcept override { return m_pObject != nullptr; }

		MemberFunHandler(MemberFunHandler const&) = delete;
		MemberFunHandler(MemberFunHandler&&) = delete;
		MemberFunHandler& operator=(MemberFunHandler const&) = delete;
		MemberFunHandler& operator=(MemberFunHandler&&) = delete;

	private:
		T* m_pObject;
		MemFnType m_MemFn;
	};

	template<typename T, typename ParamType = void>
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
	template<typename T>
	class MemberFunHandlerConst<T, void> final : public IListenerHandler<void>
	{
	public:
		using MemFnType = void (T::*)() const;

		MemberFunHandlerConst(ListenerHandle const& handle, T const* obj, MemFnType fn) :
			IListenerHandler{ handle },
			m_Object{ obj },
			m_MemFn{ fn } {
		}

		void Invoke() const noexcept override
		{
			(m_Object->*m_MemFn)();
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


	template<typename T, typename EventType = void>
	struct BindingConstMemFn final
	{
		using MemFnType = void (T::*)(EventType const&) const;

		explicit BindingConstMemFn(void (T::* memFunc)(EventType const&) const, T const* instance, void* owner = nullptr) :
			instance{ instance }, memFn{ memFunc }, owner{ owner } { }

		T const* instance;
		MemFnType memFn;
		void* owner;
	};
	template<typename T>
	struct BindingConstMemFn<T, void> final
	{
		using MemFnType = void (T::*)() const;

		explicit BindingConstMemFn(MemFnType mem, T const* ins, void* own = nullptr)
			: instance{ ins }, memFn{ mem }, owner{ own } {
		}

		T const* instance;
		MemFnType memFn;
		void* owner;
	};
	template<typename T, typename EventType = void>
	struct BindingMemFn final
	{
		using MemFnType = void (T::*)(EventType const&);

		explicit BindingMemFn(void (T::* mem)(EventType const&), T* ins, void* own = nullptr) :
			instance{ ins }, memFn{ mem }, owner{ own } { }

		T* instance;
		MemFnType memFn;
		void* owner;
	};
	template<typename T>
	struct BindingMemFn<T, void> final
	{
		using MemFnType = void (T::*)();

		explicit BindingMemFn(MemFnType mem, T* ins, void* own = nullptr)
			: instance{ ins }, memFn{ mem }, owner{ own } { }

		T* instance;
		MemFnType memFn;
		void* owner;
	};

	template<typename Callable>
	struct BindingCallable final
	{
		explicit BindingCallable(Callable&& call, void* own = nullptr) :
			callable{ std::move(call) }, owner{ own } { }

		Callable callable;
		void* owner;
	};

	// Const member function with ParamType
	template<typename T, typename ParamType>
	auto Bind(void (T::* memFn)(ParamType const&) const, T const* instance, void* owner = nullptr)
	{
		return BindingConstMemFn<T, ParamType>(memFn, instance, owner);
	}

	// Const member function with no param (ParamType = void)
	template<typename T>
	auto Bind(void (T::* memFn)() const, T const* instance, void* owner = nullptr)
	{
		return BindingConstMemFn<T, void>(memFn, instance, owner);
	}

	// Non-const member function with ParamType
	template<typename T, typename ParamType>
	auto Bind(void (T::* memFn)(ParamType const&), T* instance, void* owner = nullptr)
	{
		return BindingMemFn<T, ParamType>(memFn, instance, owner);
	}

	// Non-const member function with no param (ParamType = void)
	template<typename T>
	auto Bind(void (T::* memFn)(), T* instance, void* owner = nullptr)
	{
		return BindingMemFn<T, void>(memFn, instance, owner);
	}

	template<typename ParamType = void, typename Callable>
		requires (std::is_void_v<ParamType> && CallableNoParam<Callable>)
	auto Bind(Callable&& callable, void* owner = nullptr)
	{
		static_assert(std::is_void_v<ParamType>);
		return BindingCallable<Callable>(std::move(callable), owner);
	}
	template<typename ParamType, typename Callable>
		requires (!std::is_void_v<ParamType> && CallableWithParam<ParamType, Callable>)
	auto Bind(Callable&& callable, void* owner = nullptr)
	{
		return BindingCallable<Callable>(std::move(callable), owner);
	}
}

#endif