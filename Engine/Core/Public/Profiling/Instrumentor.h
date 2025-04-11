// ReSharper disable IdentifierTypo
#ifndef MAUCOR_INSTRUMENTOR_H
#define MAUCOR_INSTRUMENTOR_H

#include "CorePCH.h"

#include "Profiling/InstrumentorTimer.h"

// must be included before optick.h to control macros
#include "Config/EngineConfig.h"
#if USE_OPTICK_LIBRARY
	#include <Optick.h>
#endif

namespace MauCor
{
	struct ProfileResult final
	{
		std::string name;
		long long start;
		long long end;
		std::thread::id threadID;
	};

	struct InstrumentationSession final
	{
		std::string name;
	};

    class Instrumentor final : public Singleton<Instrumentor>
    {
    public:
		void BeginSession(std::string const& name, std::string const& filepath, size_t reserveSize = 100'000);

		void WriteProfile(ProfileResult const& result, bool isFunction);

		void EndSession();

		Instrumentor(Instrumentor const&) = delete;
		Instrumentor(Instrumentor&&) = delete;
		Instrumentor& operator=(Instrumentor const&) = delete;
		Instrumentor& operator=(Instrumentor&&) = delete;
	private:
		friend class Singleton<Instrumentor>;
		Instrumentor() = default;
		virtual ~Instrumentor() override;

		mutable std::mutex m_Mutex;

		std::unique_ptr<InstrumentationSession> m_CurrentSession{ nullptr };
    	std::string m_Buffer;

		size_t BUFFER_RESERVE_SIZE{};
    	size_t BUFFER_FLUSH_THRESHOLD{};

		std::ofstream m_OutputStream{  };

		void WriteHeader();
		void WriteFooter();
    };

#ifdef ENABLE_PROFILER
	#define CONCAT(x, y) x ## y
	#define C(x, y) CONCAT(x, y)

	#if USE_OPTICK_LIBRARY
		#define ME_PROFILE_BEGIN_SESSION(name, filepath, ...)
		#define ME_PROFILE_END_SESSION()
		#define ME_PROFILE_SCOPE(name)  OPTICK_EVENT(name)
		#define ME_PROFILE_FUNCTION() OPTICK_EVENT()

		#define ME_PROFILE_THREAD(name) OPTICK_THREAD(name)
		#define ME_PROFILE_FRAME(name) OPTICK_FRAME("MainThread")

	#else
		// Name, filepath, reserve size - allows you to specifiy a reserve size for the buffer to prevent big resizes.
		#define ME_PROFILE_BEGIN_SESSION(name, filepath, ...) MauCor::Instrumentor::GetInstance().BeginSession(name, filepath, __VA_ARGS__)
		#define ME_PROFILE_END_SESSION() MauCor::Instrumentor::GetInstance().EndSession()
		#define ME_PROFILE_SCOPE(name) MauCor::InstrumentorTimer C(timer, __LINE__) { name, false }
		#define ME_PROFILE_FUNCTION() MauCor::InstrumentorTimer C(timer, __LINE__) { __FUNCTION__, true }

		#define ME_PROFILE_THREAD(name)
	#endif
#else
	#define ME_PROFILE_BEGIN_SESSION(name, filepath, ...)
	#define ME_PROFILE_END_SESSION()
	#define ME_PROFILE_FUNCTION()
	#define ME_PROFILE_SCOPE(name)

	#define ME_PROFILE_THREAD(name)
#endif
}

#endif