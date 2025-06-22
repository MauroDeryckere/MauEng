#ifndef MAUCOR_NULLLOGGER_H
#define MAUCOR_NULLLOGGER_H

#include "Logger/Logger.h"

namespace MauCor
{
	class NullLogger final : public Logger
	{
	public:
		NullLogger() = default;
		virtual ~NullLogger() override = default;

		NullLogger(NullLogger const&) = delete;
		NullLogger(NullLogger&&) = delete;
		NullLogger& operator=(NullLogger const&) = delete;
		NullLogger& operator=(NullLogger&&) = delete;

	protected:

	private:
		virtual void LogInternal(LogPriority priority, std::string_view const category, std::string_view const message) override {}
	};
}

#endif