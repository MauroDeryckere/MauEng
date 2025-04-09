#ifndef MAUCOR_LOGCATEGORIES_H
#define MAUCOR_LOGCATEGORIES_H

namespace MauCor
{
	enum class LogPriority : uint8_t
	{
		Trace,
		Info,
		Debug,
		Warn,
		Error,
		Fatal
	};

	enum class LogCategory : uint8_t
	{
		Core,
		Engine,
		Renderer,
		Game
	};
}

#endif