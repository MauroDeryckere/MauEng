#include "CustomPlayerClass.h"

namespace MauGam
{
	void PlayerClass::Tick()
	{
		Player::Tick();

		static bool exexOnce{ false };
		if (!exexOnce)
		{
			ME_LOG_DEBUG(LogGame, "Custom tick function for player class! ");
		}
		exexOnce = true;
	}

	void PlayerClass::OnActionExecuted(MauEng::InputEvent const& event) noexcept
	{
		ME_LOG_DEBUG(LogGame, "Action executed for player class! Action: {}, Player ID: {}", event.action, PlayerID());
	}
}
