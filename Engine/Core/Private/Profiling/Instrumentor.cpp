#include "Profiling/Instrumentor.h"

namespace MauCor
{
	void Instrumentor::BeginSession(std::string const& name, std::string const& filepath)
    {
        std::cout << filepath << "\n";
        m_OutputStream.open(filepath);

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
        m_OutputStream.close();

		m_CurrentSession = nullptr;

        m_ProfileCount = 0;
    }

	void Instrumentor::WriteProfile(ProfileResult const& result)
    {
        if (m_ProfileCount++ > 0)
        {
            m_OutputStream << ",";
        }

        std::string name{ result.name };
        std::replace(name.begin(), name.end(), '"', '\'');

        m_OutputStream << "{";
        m_OutputStream << R"("cat":"function",)";
        m_OutputStream << "\"dur\":" << (result.end - result.start) << ',';
        m_OutputStream << R"("name":")" << name << "\",";
        m_OutputStream << R"("ph":"X",)";
        m_OutputStream << "\"pid\":0,";
        m_OutputStream << "\"tid\":" << result.threadID << ",";
        m_OutputStream << "\"ts\":" << result.start;
        m_OutputStream << "}";

        m_OutputStream.flush();
    }

	Instrumentor::~Instrumentor()
	{
        EndSession();
	}

	void Instrumentor::WriteHeader()
    {
        m_OutputStream << R"({"otherData": {},"traceEvents":[)";
        m_OutputStream.flush();
    }

	void Instrumentor::WriteFooter()
    {
        m_OutputStream << "]}";
        m_OutputStream.flush();
    }

}
