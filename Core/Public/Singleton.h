#ifndef MAUCOR_SINGLETON_H
#define MAUCOR_SINGLETON_H

namespace MauCor
{
	template <typename T>
	class Singleton
	{
	public:
		[[nodiscard]] static T& GetInstance() noexcept
		{
			static T instance{};
			return instance;
		}

		virtual ~Singleton() = default;
		Singleton(Singleton const&) = delete;
		Singleton(Singleton&&) = delete;
		Singleton& operator=(Singleton const&) = delete;
		Singleton& operator=(Singleton&&) = delete;

	protected:
		Singleton() = default;
	};
}

#endif