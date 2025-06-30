#ifndef MAUENG_IMGUILOGGER_H
#define MAUENG_IMGUILOGGER_H

#include "Logger/Logger.h"

namespace MauEng
{
	class ImGUILogger final : public MauCor::Logger
	{
	public:
		struct LogMessage final
		{
			MauCor::ELogPriority priority;
			std::string category;
			std::string message;
		};

		explicit ImGUILogger() = default;
		virtual ~ImGUILogger() override = default;

		void Clear() noexcept { m_LogBuffer.clear(); }

		[[nodiscard]] std::vector<LogMessage> const& GetLogBuffer() const noexcept { return m_LogBuffer; }
		[[nodiscard]] bool GetAutoScroll() const noexcept { return m_AutoScroll; }

		void SetAutoScroll(bool val) noexcept { m_AutoScroll = val; }

		ImGUILogger(ImGUILogger const&) = delete;
		ImGUILogger(ImGUILogger&&) = delete;
		ImGUILogger& operator=(ImGUILogger const&) = delete;
		ImGUILogger& operator=(ImGUILogger&&) = delete;

	protected:
		virtual void LogInternal(MauCor::ELogPriority priority, std::string_view const category, std::string_view const message) override;

	private:
		std::vector<LogMessage> m_LogBuffer{ };
		bool m_AutoScroll{ true };
		static constexpr size_t MAX_LOG_MESSAGES{ 1000 };
	};
}

#endif