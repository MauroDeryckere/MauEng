#ifndef MAUCOR_CONSOLELOGGER_H
#define MAUCOR_CONSOLELOGGER_H

#include "Logger.h"

namespace MauCor
{
	class ConsoleLogger final : public Logger
	{
	public:
		
	private:
		virtual void LogInternal(LogPriority priority, std::string const& message) override;
	};
}

#endif