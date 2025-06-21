#ifndef MAUENG_INPUTEVENT_H
#define MAUENG_INPUTEVENT_H

#include <string>

namespace MauEng
{
	struct InputEvent final
	{
		uint32_t const playerID;
		std::string const action;
	};
}

#endif