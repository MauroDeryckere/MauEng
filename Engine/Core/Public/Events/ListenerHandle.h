#ifndef MAUCOR_LISTENERHANDLE_H
#define MAUCOR_LISTENERHANDLE_H

namespace MauCor
{
	struct ListenerHandle final
	{
		uint32_t id{ 0 };
		void* const owner{ nullptr };

		bool constexpr operator==(ListenerHandle const& other) const noexcept
		{
			return id == other.id and owner == other.owner;
		}
		bool constexpr operator!=(ListenerHandle const& other) const noexcept
		{
			return !(*this == other);
		}
	};
}

#endif