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

	enum class ELogCategory : uint8_t
	{
		Core,
		Engine,
		Renderer,
		Game
	};

	class LogCategory final
	{
		public:
			explicit constexpr LogCategory(char const* name) noexcept 
				: m_Name{ name }
			{ }
		constexpr char const* GetName() const noexcept { return m_Name; }

		private:
			char const* m_Name;
	};
}

#define DEFINE_LOG_CATEGORY(CategoryName) \
    MauCor::LogCategory CategoryName(#CategoryName);

#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName) \
    extern MauCor::LogCategory CategoryName;

MauCor::ELogPriority constexpr Trace{ MauCor::ELogPriority::Trace };
MauCor::ELogPriority constexpr Info{ MauCor::ELogPriority::Info };
MauCor::ELogPriority constexpr Debug{ MauCor::ELogPriority::Debug };
MauCor::ELogPriority constexpr Warn{ MauCor::ELogPriority::Warn };
MauCor::ELogPriority constexpr Error{ MauCor::ELogPriority::Error };
MauCor::ELogPriority constexpr Fatal{ MauCor::ELogPriority::Fatal };

#endif