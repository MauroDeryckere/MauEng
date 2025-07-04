#ifndef MAUENG_PLAYER_H
#define MAUENG_PLAYER_H

#include "events/inputEvent.h"

namespace MauEng
{
	uint32_t constexpr INVALID_PLAYER_ID{ UINT32_MAX };

	class Player
	{
	public:
		[[nodiscard]] uint32_t PlayerID() const noexcept { return m_PlayerID; }
		[[nodiscard]] bool IsGamepadConnected() const noexcept;

		[[nodiscard]] std::string const& GetGamepadMappingContext() const noexcept;
		[[nodiscard]] std::string const& GetKeyboardMappingContext() const noexcept;

		[[nodiscard]] bool IsActionExecuted(std::string const& action) const noexcept;

		void SetMappingContext(std::string const& mappingContext) const noexcept;
		void SetGamepadMappingContext(std::string const& mappingContext) const noexcept;
		void SetKeyboardMappingContext(std::string const& mappingContext) const noexcept;

		[[nodiscard]] std::pair<float, float> GetLeftJoystick() const noexcept;
		[[nodiscard]] std::pair<float, float> GetDeltaLeftJoystick() const noexcept;

		[[nodiscard]] std::pair<float, float> GetRightJoystick() const noexcept;
		[[nodiscard]] std::pair<float, float> GetDeltaRightJoystick() const noexcept;

		[[nodiscard]] float GetLeftTrigger() const noexcept;
		[[nodiscard]] float GetDeltaLeftTrigger() const noexcept;

		[[nodiscard]] float GetRightTrigger() const noexcept;
		[[nodiscard]] float GetDeltaRightTrigger() const noexcept;


		[[nodiscard]] std::pair<float, float> GetMousePosition() const noexcept;
		[[nodiscard]] std::pair<float, float> GetDeltaMouseMovement() const noexcept;
		[[nodiscard]] std::pair<float, float> GetDeltaMouseScroll() const noexcept;

		virtual void Tick();

		virtual void OnActionExecuted(InputEvent const& event) noexcept { }

		explicit Player(uint32_t id);
		virtual ~Player();

	protected:
		Player(Player const&) = delete;
		Player(Player&&) = delete;
		Player& operator=(Player const&) = delete;
		Player& operator=(Player&&) = delete;

	private:
		uint32_t m_PlayerID{ INVALID_PLAYER_ID };
	};
}

#endif