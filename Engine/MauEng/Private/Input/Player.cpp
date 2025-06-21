#include "Input/Player.h"
#include "Input/InputManager.h"

namespace MauEng
{
	Player::Player(uint32_t id)
		: m_PlayerID(id)
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);

		INPUT_MANAGER.m_InputDelegateDelayed += MauCor::Bind(&Player::OnActionExecuted, this, this);
	}
	Player::~Player()
	{
		// necessary unless we add a dynamic system to track this
		INPUT_MANAGER.m_InputDelegateDelayed -= this;
	}

	bool Player::IsGamepadConnected() const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.HasGamepadForPlayerID(m_PlayerID);
	}

	std::string const& Player::GetGamepadMappingContext() const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.GetGamepadMappingContext(m_PlayerID);
	}

	std::string const& Player::GetKeyboardMappingContext() const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.GetKeyboardMappingContext(m_PlayerID);
	}

	bool Player::IsActionExecuted(std::string const& action) const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.IsActionExecuted(action, m_PlayerID);
	}

	void Player::SetMappingContext(std::string const& mappingContext) const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		INPUT_MANAGER.SetMappingContext(mappingContext, m_PlayerID);
	}

	void Player::SetGamepadMappingContext(std::string const& mappingContext) const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		INPUT_MANAGER.SetGamepadMappingContext(mappingContext, m_PlayerID);
	}

	void Player::SetKeyboardMappingContext(std::string const& mappingContext) const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		INPUT_MANAGER.SetKeyboardMappingContext(mappingContext, m_PlayerID);
	}

	std::pair<float, float> Player::GetLeftJoystick() const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.GetLeftJoystick(m_PlayerID);
	}

	std::pair<float, float> Player::GetDeltaLeftJoystick() const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.GetDeltaLeftJoystick(m_PlayerID);
	}

	std::pair<float, float> Player::GetRightJoystick() const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.GetRightJoystick(m_PlayerID);
	}

	std::pair<float, float> Player::GetDeltaRightJoystick() const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.GetDeltaRightJoystick(m_PlayerID);
	}

	float Player::GetLeftTrigger() const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.GetLeftTrigger(m_PlayerID);
	}

	float Player::GetDeltaLeftTrigger() const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.GetDeltaLeftTrigger(m_PlayerID);
	}

	float Player::GetRightTrigger() const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.GetRightTrigger(m_PlayerID);
	}

	float Player::GetDeltaRightTrigger() const noexcept
	{
		ME_ENGINE_ASSERT(m_PlayerID != INVALID_PLAYER_ID);
		return INPUT_MANAGER.GetDeltaRightTrigger(m_PlayerID);
	}

	std::pair<float, float> Player::GetMousePosition() const noexcept
	{
		return INPUT_MANAGER.GetMousePosition();
	}

	std::pair<float, float> Player::GetDeltaMouseMovement() const noexcept
	{
		return INPUT_MANAGER.GetDeltaMouseMovement();
	}

	std::pair<float, float> Player::GetDeltaMouseScroll() const noexcept
	{
		return INPUT_MANAGER.GetDeltaMouseScroll();
	}

	void Player::Tick()
	{

	}
}
