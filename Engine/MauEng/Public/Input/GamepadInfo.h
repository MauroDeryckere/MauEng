#ifndef MAUENG_GAMEPADINFO_H
#define MAUENG_GAMEPADINFO_H

#include "SDL3/SDL_gamepad.h"

namespace MauEng
{
	struct GamepadInfo final
	{
		enum class ActionType : uint8_t
		{
			Down,
			Up,
			Held,

			// Action Type axis -> axis in input field
			AxisHeld,
			AxisMoved,
			AxisReleased,
			AxisStartHeld,

			COUNT
		};

		union ButtonAxis
		{
			SDL_GamepadButton button{ SDL_GAMEPAD_BUTTON_INVALID };
			SDL_GamepadAxis axis;
		};

		ButtonAxis input{};

		ActionType type{};
	};

}

#endif