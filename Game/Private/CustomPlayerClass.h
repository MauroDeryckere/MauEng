#ifndef MAUGAM_PLAYERCLASS_H
#define MAUGAM_PLAYERCLASS_H

#include "Input/InputManager.h"

namespace MauGam
{
	class PlayerClass final : public MauEng::Player
	{
	public:
		PlayerClass(uint32_t playerID) :
			Player{ playerID } { }

		virtual ~PlayerClass() override = default;

	protected:
		void Tick() override;

	private:
	};
}

#endif