#ifndef MAUCOR_OPTICKPROFILER_H
#define MAUCOR_OPTICKPROFILER_H

#include "Profiling/Profiler.h"

namespace MauCor
{
	class OptickProfiler final : public Profiler
	{
	public:
		OptickProfiler() = default;
		virtual ~OptickProfiler() override = default;

		virtual void BeginSessionInternal(std::string const& name, size_t reserveSize = 100'000) override;

		virtual void WriteProfile(ProfileResult const& result, bool isFunction) override;
		virtual void WriteProfile(std::string const& name) override;

		virtual void EndSession() override;

		OptickProfiler(OptickProfiler const&) = delete;
		OptickProfiler(OptickProfiler&&) = delete;
		OptickProfiler& operator=(OptickProfiler const&) = delete;
		OptickProfiler& operator=(OptickProfiler&&) = delete;
	private:
	};
}

#endif
