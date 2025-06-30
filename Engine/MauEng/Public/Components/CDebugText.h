#ifndef MAUENG_CDEBUGTEXT_H
#define MAUENG_CDEBUGTEXT_H

#include <glm/glm.hpp>
#include <string>

namespace MauEng
{
	enum class HorizontalAlignment : uint8_t
	{
		Left = 0,
		Center = 1,
		Right = 2
	};
	enum class VerticalAlignment : uint8_t
	{
		Top = 0,
		Middle = 1,
		Bottom = 2
	};

	struct CDebugText final
	{
		std::string text{ "NO TEXT" };
		glm::vec4 colour{ 1.f, 1.f, 1.f, 1.f };

		// Default font size
		float baseFontSize{ 20.f };
		// Minimum font size when scaling
		float minFontSize{ 10.f };
		// Maximum font size when scaling
		float maxFontSize{ 40.f };

		bool scaleWithDistance{ true };

		HorizontalAlignment hAlign{ HorizontalAlignment::Center };
		VerticalAlignment vAlign{ VerticalAlignment::Middle };
	};
}

#endif