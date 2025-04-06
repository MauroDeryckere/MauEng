#ifndef MAUENG_ENGINECONFIG_H
#define MAUENG_ENGINECONFIG_H

namespace MauEng
{
	// Debug output colours

	// White (default)
	auto constexpr LOG_COLOR_RESET{ "\033[0m" };
	// Red
	auto constexpr LOG_COLOR_ERROR{ "\033[1;31m" };
	// Yellow
	auto constexpr LOG_COLOR_WARNING{ "\033[1;33m" };
	// Cyan
	auto constexpr LOG_COLOR_INFO{ "\033[1;36m" };
	// Gray
	auto constexpr LOG_COLOR_GENERAL{ "\033[1;90m" };
	// Blue
	auto constexpr LOG_COLOR_CATEGORY{ "\033[1;34m" };
}

#endif