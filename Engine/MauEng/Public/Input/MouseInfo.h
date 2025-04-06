#ifndef MAUENG_MOUSEINFO_H
#define MAUENG_MOUSEINFO_H

namespace MauEng
{
	struct MouseInfo final
	{
		enum class ActionType : uint8_t
		{
			Down,
			Up,
			Held,
			Moved,
			Scrolled,

			EnteredWindow,
			LeftWindow,

			COUNT
		};

		uint8_t button{ 0 };
		ActionType type{};
	};
}

#endif