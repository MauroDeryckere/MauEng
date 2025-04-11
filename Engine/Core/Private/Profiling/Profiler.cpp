#include "Profiling/Profiler.h"

namespace MauCor
{
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
			ME_LOG_WARN(LogCategory::Core, "Existing file deleted: {}", filepath);
		}
	}
}
