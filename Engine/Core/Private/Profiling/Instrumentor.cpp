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

        std::string functionName{ result.name };
	    if (isFunction)
	    {
            CleanUpFunctionName(functionName);
	    }

        std::lock_guard lock(m_Mutex);

        m_Buffer += ",";
        m_Buffer += "{";
        m_Buffer += R"("cat":")" + std::string(isFunction ? "function" : "scope") + R"(",)";
        m_Buffer += R"("dur":)" + std::to_string(result.end - result.start) + ",";
        m_Buffer += R"("name":")";
        m_Buffer += functionName;
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
        m_Buffer += "{}";
        m_OutputStream << m_Buffer;

        m_Buffer.clear();
    }

	void Instrumentor::WriteFooter()
    {
        m_Buffer += "]}";
    }

	void Instrumentor::CleanUpFunctionName(std::string& name)
	{
        size_t const firstSpace{ name.find_first_of(' ') };
        if (firstSpace != std::string::npos)
        {
            name = name.substr(firstSpace + 1);
        }

        // Remove the class or namespace prefix (if any)
        size_t const pos{ name.find("::") };
        if (pos != std::string::npos)
        {
        	// Skip past the "::"
            name = name.substr(pos + 2);
        }
	}
}
