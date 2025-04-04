#include "SDLWindow.h"

#include "ServiceLocator.h"

namespace MauEng
{
	SDLWindow::SDLWindow()
	{
		if (not SDL_Init(SDL_INIT_VIDEO))
		{
			throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
		}

		window = SDL_CreateWindow(
			windowTitle.c_str(),
			width,
			height,
			SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
		);

		if (!window)
		{
			throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
		}
	}

	SDLWindow::~SDLWindow()
	{
		if (window)
		{
			SDL_DestroyWindow(window);
			SDL_Quit();
		}	
	}

	void SDLWindow::Initialize()
	{
		SDL_SetEventFilter(HandleWindowEvent, this);
	}

	bool SDLWindow::HandleWindowEvent(void* userdata, SDL_Event* event)
	{
		if (event->type == SDL_EVENT_WINDOW_RESIZED)
		{
			if (!userdata)
			{
				throw std::runtime_error("Failed to get window user pointer!");
			}

			SDLWindow* winClassPtr{ static_cast<SDLWindow*>(userdata) };

			winClassPtr->width = static_cast<uint16_t>(event->window.data1);
			winClassPtr->height = static_cast<uint16_t>(event->window.data2);

			ServiceLocator::GetRenderer().ResizeWindow();

			// TODO reflect width / heigh change in the camera
			// -> ServiceLocator::GetSceneManager::UpdateActiveCameraAspectRatio(width, height);
		}

		return true;
	}
}
