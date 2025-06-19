#ifndef MAUENG_GAMEPADINFO_H
#define MAUENG_GAMEPADINFO_H

#include "SDL3/SDL_gamepad.h"

namespace MauEng
{
	//TODO joystick / trigger
	struct GamepadInfo final
	{
		enum class ActionType : uint8_t
		{
			Down,
			Up,
			Held,

			COUNT
		};

		SDL_GamepadButton button{};
		//TODO
		//SDL_GamepadAxis axis;

		ActionType type{ };
	};

}

#endif