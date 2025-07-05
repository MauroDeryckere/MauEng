#ifndef MAUCOR_LOGCATEGORIES_H
#define MAUCOR_LOGCATEGORIES_H

namespace MauCor
{
	enum class ELogPriority : uint8_t
	{
		Trace,
		Info,
		Debug,
		Warn,
		Error,
		Fatal
	};

	class LogCategory final
	{
	public:
		explicit constexpr LogCategory(char const* name, MauCor::ELogPriority priority = MauCor::ELogPriority::Trace) noexcept
			: m_Name{ name },
			  m_Priority{ priority }
		{ }

		[[nodiscard]] constexpr char const* GetName() const noexcept { return m_Name; }
		[[nodiscard]] constexpr MauCor::ELogPriority GetPriority() const noexcept { return m_Priority; }

		void SetPriority(MauCor::ELogPriority priority) noexcept;

	private:
		char const* m_Name;
		MauCor::ELogPriority m_Priority;
	};
}


// should only be placed in one cpp file
#define DEFINE_LOG_CATEGORY(CategoryName, ...) \
    MauCor::LogCategory CategoryName{ #CategoryName, __VA_ARGS__ };

#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName) \
    extern MauCor::LogCategory CategoryName;


DECLARE_LOG_CATEGORY_EXTERN(LogEngine);
DECLARE_LOG_CATEGORY_EXTERN(LogRenderer);
DECLARE_LOG_CATEGORY_EXTERN(LogCore);
DECLARE_LOG_CATEGORY_EXTERN(LogGame);
DECLARE_LOG_CATEGORY_EXTERN(LogAudio);

MauCor::ELogPriority constexpr Trace{ MauCor::ELogPriority::Trace };
MauCor::ELogPriority constexpr Info{ MauCor::ELogPriority::Info };
MauCor::ELogPriority constexpr Debug{ MauCor::ELogPriority::Debug };
MauCor::ELogPriority constexpr Warn{ MauCor::ELogPriority::Warn };
MauCor::ELogPriority constexpr Error{ MauCor::ELogPriority::Error };
MauCor::ELogPriority constexpr Fatal{ MauCor::ELogPriority::Fatal };

#endif