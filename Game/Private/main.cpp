#include <iostream>

#include "Engine.h"

#include "Scene/SceneManager.h"
#include "DemoScene.h"

void InitDemoScene();

// SDL needs to be able to overwrite main depending on the platform here
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
int main(int, char* [])
{
	#ifdef _WIN32
		if constexpr (USE_IMGUI or ENABLE_FILE_LOGGING)
		{
			HWND hWnd{ GetConsoleWindow() };
			if (hWnd != nullptr) {
				ShowWindow(hWnd, SW_HIDE);
			}
		}
		else
		{
			HWND hWnd{ GetConsoleWindow() };
			if (hWnd != nullptr) {
				ShowWindow(hWnd, SW_SHOW);
			}
		}
	#endif

	using namespace MauEng;
	Engine engine{};

	engine.Run(InitDemoScene);

	return 0;
}

void InitDemoScene()
{
	using namespace MauGam;

	MauEng::SceneManager::GetInstance().LoadScene(std::make_unique<DemoScene>());
}