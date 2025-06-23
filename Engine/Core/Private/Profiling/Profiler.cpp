#include "Profiling/Profiler.h"

namespace MauCor
{
	void Profiler::BeginSession(std::string const& name, char const* path, size_t reserveSize)
	{
		fileName = path;
		BeginSessionInternal(name, reserveSize);
	}

	void Profiler::Start(char const* path)
	{
		if (isProfiling)
		{
			ME_LOG_INFO(MauCor::ELogCategory::Core, "Already profiling {}", fileName);
		}
		else
		{
			fileName = path;
			fileName += std::to_string(numExecutedProfiles);

			ME_LOG_INFO(MauCor::ELogCategory::Core, "Beginning profile session {}", fileName);
			BeginSessionInternal(fileName);
			isProfiling = true;
		}
	}

	void Profiler::Update()
	{
		if (isProfiling)
		{
			++profiledFrames;
		}
		if (profiledFrames == MauEng::NUM_FRAMES_TO_PROFILE)
		{
			++numExecutedProfiles;
			profiledFrames = 0;
			isProfiling = false;

			EndSession();

			ME_LOG_INFO(MauCor::ELogCategory::Core, "Ending profile session {}", fileName);
		}
	}

	void Profiler::FixFilePath(char const* filepath)
	{
		std::filesystem::path const dir{ std::filesystem::path(filepath).parent_path() };
		if (!std::filesystem::exists(dir))
		{
			std::filesystem::create_directories(dir);
		}

		if (std::filesystem::exists(filepath))
		{
			std::filesystem::remove(filepath);
			ME_LOG_WARN(ELogCategory::Core, "Existing file deleted: {}", filepath);
		}
	}
}
