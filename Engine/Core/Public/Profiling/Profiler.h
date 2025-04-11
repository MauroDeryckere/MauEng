#ifndef MAUCOR_PROFILER_H
#define MAUCOR_PROFILER_H

#include <string>
#include <thread>

namespace MauCor
{
	struct ProfileResult final
	{
		std::string name;
		long long start;
		long long end;
		std::thread::id threadID;
	};

	class Profiler
	{
	public:
		virtual ~Profiler() = default;

		virtual void BeginSession(std::string const& name, std::string const& filepath, size_t reserveSize = 100'000) = 0;

		virtual void WriteProfile(ProfileResult const& result, bool isFunction) = 0;
		virtual void WriteProfile(std::string const& name) = 0;

		virtual void EndSession() = 0;

		void Start(char const* path);
		void Update();

		static void FixFilePath(char const* filepath);

		Profiler(Profiler const&) = delete;
		Profiler(Profiler&&) = delete;
		Profiler& operator=(Profiler const&) = delete;
		Profiler& operator=(Profiler&&) = delete;
	protected:
		Profiler() = default;

	private:
		uint32_t profiledFrames{ 0 };
		bool isProfiling{ false };

		uint32_t numExecutedProfiles{ 0 };

		std::string fileName;
	};
}

#endif