#ifndef MAUENG_INPUTACTION_H
#define MAUENG_INPUTACTION_H

#include "KeyboardKey.h"

#include "SDL3/SDL_keycode.h"

namespace MauEng
{

	struct KeyInfo final
	{
		enum class ActionType : uint8_t
		{
			Down,
			Up,
			Held,

			COUNT
		};

		//KeyBoardKey key{};
		SDL_Keycode key{};
		ActionType type{};
	};
}

#endif