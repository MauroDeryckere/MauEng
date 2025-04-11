#include "Profiling/GoogleProfiler.h"
#include "Asserts/AsseertsInternal.h"

namespace MauCor
{
	GoogleProfiler::~GoogleProfiler()
	{
		EndSession();
	}

	void GoogleProfiler::BeginSession(std::string const& name, std::string const& filepath, size_t reserveSize)
	{
		EndSession();

		auto fullFilepath{ std::move(filepath) };
		fullFilepath += ".json";

		FixFilePath(fullFilepath.c_str());

		m_OutputStream.open(fullFilepath);

		if (!m_OutputStream.is_open())
		{
			ME_LOG_ERROR(LogCategory::Core, "Failed to open the file: {}", fullFilepath);
			return;
		}

		m_Buffer.clear();
		m_Buffer.reserve(reserveSize);
		BUFFER_FLUSH_THRESHOLD = static_cast<size_t>(.9f * reserveSize);
		BUFFER_RESERVE_SIZE = reserveSize;

		WriteHeader();

		m_CurrentSession = std::make_unique<InstrumentationSession>(name);
	}

	void GoogleProfiler::WriteProfile(ProfileResult const& result, bool isFunction)
	{
		if (!m_CurrentSession)
		{
			return;
		}

		std::stringstream ss;
		ss << ",";
		ss << "{";
		ss << R"("cat":")" << (isFunction ? "function" : "scope") << R"(",)";
		ss << R"("dur":)" << (result.end - result.start) << ",";
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

	void GoogleProfiler::WriteProfile(std::string const& name)
	{
		ME_LOG_ERROR(LogCategory::Core, "Incorrect profiler function for google profiler used.");
		ME_CORE_CHECK(false);
	}

	void GoogleProfiler::EndSession()
	{
		if (m_CurrentSession)
		{
			WriteFooter();

			m_OutputStream << m_Buffer;
			m_OutputStream.close();

			m_CurrentSession = nullptr;
		}
	}

	void GoogleProfiler::WriteHeader()
	{
		m_OutputStream << R"({"otherData": {},"traceEvents":[{})";
		m_OutputStream.flush();
	}

	void GoogleProfiler::WriteFooter()
	{
		m_Buffer += "]}";
	}
}
