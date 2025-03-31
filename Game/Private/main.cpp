#include <iostream>

#include "Engine.h"
#include "GameScene.h"
#include "Scene/SceneManager.h"

void InitDemoScene();

int main()
{
	using namespace MauEng;
	Engine engine{};

	engine.Run(InitDemoScene);

	return 0;
}

void InitDemoScene()
{
	using namespace MauGam;
	MauEng::SceneManager::GetInstance().LoadScene(std::make_unique<GameScene>());
}