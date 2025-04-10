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
        BUFFER_RESERVE_SIZE = reserveSize;

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
    }

	void Instrumentor::WriteProfile(ProfileResult const& result, bool isFunction)
    {
	    if (!m_CurrentSession)
	    {
            return;
	    }

        std::stringstream ss;
        ss << ",";
        ss << "{";
        ss << R"("cat":")" << (isFunction ? "function" : "scope") <<  R"(",)";
        ss << R"("dur":)" <<  (result.end - result.start) << ",";
        ss << R"("name":")";
        ss << result.name;
        ss << R"(",)";
        ss << R"("ph":"X",)";
        ss << R"("pid":0,)";
        ss << R"("tid":)" << std::hash<std::thread::id>{}(result.threadID) << ",";
        ss << R"("ts":)" << result.start;
        ss << "}";

        std::lock_guard lock(m_Mutex);
        m_Buffer += ss.str();
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
		m_OutputStream << R"({"otherData": {},"traceEvents":[{})";
        m_OutputStream.flush();
    }

	void Instrumentor::WriteFooter()
    {
        m_Buffer += "]}";
    }
}
