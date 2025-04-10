#include "Profiling/Instrumentor.h"

namespace MauCor
{
	void Instrumentor::BeginSession(std::string const& name, std::string const& filepath)
    {
        ME_LOG_ERROR(LogCategory::Core, "Beginning profile session {}", filepath);
        m_OutputStream.open(filepath);

        m_Buffer.str("");
        m_Buffer.clear();

        WriteHeader();

        m_CurrentSession = std::make_unique<InstrumentationSession>(name);
    }

	void Instrumentor::EndSession()
    {
	    if (!m_CurrentSession)
	    {
            return;
	    }

        WriteFooter();

        m_OutputStream << m_Buffer.str();
        m_OutputStream.close();

		m_CurrentSession = nullptr;

        m_ProfileCount = 0;
    }

	void Instrumentor::WriteProfile(ProfileResult const& result)
    {
        if (m_ProfileCount++ > 0)
        {
            m_Buffer << ",";
        }

        std::string_view const name{ result.name };

        m_Buffer << "{";
        m_Buffer << R"("cat":"function",)";
        m_Buffer << R"("dur":)" << (result.end - result.start) << ",";
        m_Buffer << R"("name":")";
        for (char const c : name)
        {
            m_Buffer << (c == '"' ? '\'' : c);
        }
        m_Buffer << R"(",)";
        m_Buffer << R"("ph":"X",)";
        m_Buffer << R"("pid":0,)";
        m_Buffer << R"("tid":)" << result.threadID << ",";
        m_Buffer << R"("ts":)" << result.start;
        m_Buffer << "}";
    }

	Instrumentor::~Instrumentor()
	{
        EndSession();
	}

	void Instrumentor::WriteHeader()
    {
        m_Buffer << R"({"otherData": {},"traceEvents":[)";
    }

	void Instrumentor::WriteFooter()
    {
        m_Buffer << "]}";
    }

}
