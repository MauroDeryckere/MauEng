#ifndef MAUENG_INPUTACTION_H
#define MAUENG_INPUTACTION_H

#include "KeyboardKey.h"

#include "SDL3/SDL_keycode.h"

namespace MauEng
{
	// Struct to make binding input to actions easier, is not stored anywhere
	struct KeyInfo final
	{
		enum class ActionType : uint8_t
		{
			Down,
			Up,
			Held,

			COUNT
		};

		uint32_t key{};
		ActionType type{};

	};
}

#endif