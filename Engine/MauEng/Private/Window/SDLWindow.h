#ifndef MAUENG_SDL_WINDOW_H
#define MAUENG_SDL_WINDOW_H

#include "EnginePCH.h"

#include <SDL3/SDL.h>

namespace MauEng
{
	struct SDLWindow final
	{
		SDL_Window* window{ nullptr };

		uint16_t width{ 1280 };
		uint16_t height{ 720 };

		std::string windowTitle{ "VulkanProject" };

		SDLWindow();
		~SDLWindow();
		void Initialize();
		void Destroy();

		static bool HandleWindowEvent(void* userdata, SDL_Event* event);


		SDLWindow(SDLWindow const&) = delete;
		SDLWindow(SDLWindow&&) = delete;
		SDLWindow& operator=(SDLWindow const&) = delete;
		SDLWindow& operator=(SDLWindow&&) = delete;
	};

}


#endif