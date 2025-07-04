#include "CustomPlayerClass.h"

namespace MauGam
{
	void PlayerClass::Tick()
	{
		Player::Tick();

		static bool execOnce{ false };
		if (!execOnce)
		{
			ME_LOG_DEBUG(LogGame, "Custom tick function for player class! ");
		}
		execOnce = true;
	}

	void PlayerClass::OnActionExecuted(MauEng::InputEvent const& event) noexcept
	{
		bool constexpr DEBUG_OUT_ACTIONS{ false };

		if constexpr(DEBUG_OUT_ACTIONS)
		{
			ME_LOG_DEBUG(LogGame, "Action executed for player class! Action: {}, Player ID: {}", event.action, PlayerID());
		}
	}
}
