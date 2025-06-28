#ifndef MAUCOR_LISTENERHANDLE_H
#define MAUCOR_LISTENERHANDLE_H

namespace MauCor
{
	uint32_t constexpr INVALID_LISTENER_ID{ 0 };

	struct ListenerHandle final
	{
		uint32_t id{ INVALID_LISTENER_ID };
		void* const owner{ nullptr };

		bool constexpr operator==(ListenerHandle const& other) const noexcept
		{
			return id == other.id and owner == other.owner;
		}
		bool constexpr operator!=(ListenerHandle const& other) const noexcept
		{
			return !(*this == other);
		}

		explicit operator bool() const noexcept
		{
			return id != INVALID_LISTENER_ID;
		}
	};
}

#endif