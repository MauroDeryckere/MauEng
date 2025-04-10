#include "Profiling/Instrumentor.h"

namespace MauCor
{
	void Instrumentor::BeginSession(std::string const& name, std::string const& filepath, size_t reserveSize)
    {
        ME_LOG_ERROR(LogCategory::Core, "Beginning profile session {}", filepath);
        m_OutputStream.open(filepath);

        m_Buffer.clear();
        m_Buffer.reserve(reserveSize);

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

        m_OutputStream << m_Buffer;
        m_OutputStream.close();

		m_CurrentSession = nullptr;

        m_ProfileCount = 0;
    }

	void Instrumentor::WriteProfile(ProfileResult const& result)
    {
        if (m_ProfileCount++ > 0)
        {
            m_Buffer += ",";
        }

        std::string_view const name{ result.name };

        m_Buffer += "{";
        m_Buffer += R"("cat":"function",)";
        m_Buffer += R"("dur":)" + std::to_string(result.end - result.start) + ",";
        m_Buffer += R"("name":")";
        for (char const c : name)
        {
            m_Buffer += (c == '"' ? '\'' : c);
        }
        m_Buffer += R"(",)";
        m_Buffer += R"("ph":"X",)";
        m_Buffer += R"("pid":0,)";
        m_Buffer += R"("tid":)" + std::to_string(result.threadID) + ",";
        m_Buffer += R"("ts":)" + std::to_string(result.start);
        m_Buffer += "}";

        if (m_Buffer.size() >= BUFFER_FLUSH_THRESHOLD)
        {
            m_OutputStream << m_Buffer;
            m_Buffer.clear();
        }
    }

	Instrumentor::~Instrumentor()
	{
        EndSession();
	}

	void Instrumentor::WriteHeader()
    {
        m_Buffer += R"({"otherData": {},"traceEvents":[)";
    }

	void Instrumentor::WriteFooter()
    {
        m_Buffer += "]}";
    }

}
