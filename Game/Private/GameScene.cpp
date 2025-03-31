#include "GameScene.h"

#include <iostream>

namespace MauGam
{
	void GameScene::OnLoad()
	{
		Scene::OnLoad();

		std::cout << "Demo Scene loaded! \n";
	}
}
