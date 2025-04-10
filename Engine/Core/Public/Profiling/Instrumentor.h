// ReSharper disable IdentifierTypo
#ifndef MAUCOR_INSTRUMENTOR_H
#define MAUCOR_INSTRUMENTOR_H

#include "CorePCH.h"
#include "Config/EngineConfig.h"

#include "Profiling/InstrumentorTimer.h"

namespace MauCor
{
	struct ProfileResult final
	{
		std::string name;
		long long start;
		long long end;
		uint32_t threadID;
	};

	struct InstrumentationSession final
	{
		std::string name;
	};

    class Instrumentor final : public Singleton<Instrumentor>
    {
    public:
		void BeginSession(std::string const& name, std::string const& filepath = "results.json");

		void WriteProfile(ProfileResult const& result);

		void EndSession();

		Instrumentor(Instrumentor const&) = delete;
		Instrumentor(Instrumentor&&) = delete;
		Instrumentor& operator=(Instrumentor const&) = delete;
		Instrumentor& operator=(Instrumentor&&) = delete;
	private:
		friend class Singleton<Instrumentor>;
		Instrumentor() = default;
		virtual ~Instrumentor() override;

		std::unique_ptr<InstrumentationSession> m_CurrentSession{ nullptr };
    	std::ofstream m_OutputStream{ };
        uint32_t m_ProfileCount{ 0 };

		void WriteHeader();
		void WriteFooter();
    };

#ifdef ENABLE_PROFILER
	#define CONCAT(x, y) x ## y
	#define C(x, y) CONCAT(x, y)

	#define ME_PROFILE_BEGIN_SESSION(name, filepath) MauCor::Instrumentor::GetInstance().BeginSession(name, filepath)
	#define ME_PROFILE_END_SESSION() MauCor::Instrumentor::GetInstance().EndSession()
	#define ME_PROFILE_SCOPE(name) MauCor::InstrumentorTimer C(timer, __LINE__) { name }
	#define ME_PROFILE_FUNCTION() ME_PROFILE_SCOPE(__FUNCSIG__)
#else
	#define ME_PROFILE_BEGIN_SESSION(name, filepath)
	#define ME_PROFILE_END_SESSION()
	#define ME_PROFILE_FUNCTION()
	#define ME_PROFILE_SCOPE(name)
#endif
}

#endif