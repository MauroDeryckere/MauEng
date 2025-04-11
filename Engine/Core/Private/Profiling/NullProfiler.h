#ifndef MAUCOR_NULLPROFILER_H
#define MAUCOR_NULLPROFILER_H

#include "Profiling/Profiler.h"

namespace MauCor
{
	class NullProfiler final : public Profiler
	{
	public:
		NullProfiler() = default;
		virtual ~NullProfiler() override = default;

		virtual void BeginSessionInternal(std::string const& name, std::string& filepath, size_t reserveSize = 100'000) override {}

		virtual void WriteProfile(ProfileResult const& result, bool isFunction) override {}
		virtual void WriteProfile(std::string const& name) override {}

		virtual void EndSession() override {}

		NullProfiler(NullLogger const&) = delete;
		NullProfiler(NullProfiler&&) = delete;
		NullProfiler& operator=(NullProfiler const&) = delete;
		NullProfiler& operator=(NullProfiler&&) = delete;
	private:
	};
}

#endif