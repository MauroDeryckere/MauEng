#include "Profiling/Instrumentor.h"

namespace MauCor
{
	void Instrumentor::BeginSession(std::string const& name, std::string const& filepath, size_t reserveSize)
    {
        EndSession();

        ME_LOG_INFO(LogCategory::Core, "Beginning profile session {}", filepath);

        std::filesystem::path const dir{ std::filesystem::path(filepath).parent_path() };
        if (!std::filesystem::exists(dir)) 
        {
            std::filesystem::create_directories(dir);
        }

        if (std::filesystem::exists(filepath))
        {
            std::filesystem::remove(filepath);
            ME_LOG_WARN(LogCategory::Core, "Existing file deleted: {}", filepath);
        }

        m_OutputStream.open(filepath);

        if (!m_OutputStream.is_open()) 
        {
            ME_LOG_ERROR(LogCategory::Core, "Failed to open the file: {}", filepath);
            return;
        }

        m_Buffer.clear();
        m_Buffer.reserve(reserveSize);
        BUFFER_FLUSH_THRESHOLD = static_cast<size_t>(.9f * reserveSize);

        WriteHeader();

        m_CurrentSession = std::make_unique<InstrumentationSession>(name);
    }

	void Instrumentor::EndSession()
    {
        if (m_CurrentSession)
        {
			ME_LOG_INFO(LogCategory::Core, "Ending profile session");
			WriteFooter();

            m_OutputStream << m_Buffer;
            m_OutputStream.close();

            m_CurrentSession = nullptr;
        }

        m_ProfileCount = 0;
    }

	void Instrumentor::WriteProfile(ProfileResult const& result)
    {
	    if (!m_CurrentSession)
	    {
            return;
	    }

        std::lock_guard lock(m_Mutex);

		if (m_ProfileCount > 0)
        {
            m_Buffer += ",";
        }
        ++m_ProfileCount;

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
