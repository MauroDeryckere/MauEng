#ifndef MAUCOR_GOOGLEPROFILER_H
#define MAUCOR_GOOGLEPROFILER_H

namespace MauCor
{
	struct InstrumentationSession final
	{
		std::string name;
	};

	class GoogleProfiler final : public Profiler
	{
	public:
		GoogleProfiler() = default;
		virtual ~GoogleProfiler() override;

		virtual void BeginSession(std::string const& name, std::string& filepath, size_t reserveSize = 100'000) override;

		virtual void WriteProfile(ProfileResult const& result, bool isFunction) override;
		virtual void WriteProfile(std::string const& name) override;

		virtual void EndSession() override;

		GoogleProfiler(GoogleProfiler const&) = delete;
		GoogleProfiler(GoogleProfiler&&) = delete;
		GoogleProfiler& operator=(GoogleProfiler const&) = delete;
		GoogleProfiler& operator=(GoogleProfiler&&) = delete;
	private:
		mutable std::mutex m_Mutex;

		std::unique_ptr<InstrumentationSession> m_CurrentSession{ nullptr };
		std::string m_Buffer;

		size_t BUFFER_RESERVE_SIZE{};
		size_t BUFFER_FLUSH_THRESHOLD{};

		std::ofstream m_OutputStream{  };

		void WriteHeader();
		void WriteFooter();
	};
}

#endif