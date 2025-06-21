#include "CustomPlayerClass.h"

namespace MauGam
{
	void PlayerClass::Tick()
	{
		Player::Tick();

		static bool exexOnce{ false };
		if (!exexOnce)
		{
			ME_LOG_DEBUG(MauCor::LogCategory::Game, "Custom tick function for player class! ");
		}
		exexOnce = true;
	}
}
