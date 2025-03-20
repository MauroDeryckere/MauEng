#include <iostream>

#include "Engine.h"

void LoadDemoScene();

int main()
{
	using namespace MauEng;
	Engine engine{};

	engine.Run(LoadDemoScene);

	return 0;
}

void LoadDemoScene()
{
	std::cout << "Demo Scene Loaded! \n";
}