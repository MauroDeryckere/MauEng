#include "EnginePCH.h"
#include "Input/InputManager.h"

#include <SDL3/SDL.h>

namespace MauEng
{
	InputManager::InputManager()
	{
		m_MappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
		m_MappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));

		m_ActionToGamepad.resize(4);
		m_MappedGamepadActions.resize(4);
		for (auto& actions: m_MappedGamepadActions)
		{
			actions.resize(static_cast<size_t>(GamepadInfo::ActionType::COUNT));
		}
		m_ExecutedActions.resize(4);

		if constexpr (SKIP_CONTROLLER_INPUT_PLAYER_ID_0)
		{
			m_AvailablePlayerIDs.pop_back();
		}

		int numGamepads{ 0 };
		SDL_JoystickID* gamepadIDs{ SDL_GetGamepads(&numGamepads) };
		if (gamepadIDs)
		{
			for (int i{ 0 }; i < numGamepads; ++i)
			{
				if (m_AvailablePlayerIDs.empty())
				{
					break;
				}

				SDL_JoystickID const instanceID{ gamepadIDs[i] };
				if (SDL_IsGamepad(instanceID))
				{
					SDL_Gamepad* gamepad{ SDL_OpenGamepad(instanceID) };
					ME_ENGINE_ASSERT(gamepad);

					if (gamepad)
					{
						m_Gamepads.emplace_back(gamepad, m_AvailablePlayerIDs.back());
						m_AvailablePlayerIDs.pop_back();

						bool const result{ SDL_SetGamepadPlayerIndex(gamepad, static_cast<int>(m_Gamepads.back().playerID)) };
						ME_ENGINE_ASSERT(result);

						ME_LOG_INFO(
							MauCor::LogCategory::Engine,
							"Connected controller at input system init: {} player ID: {}",
							SDL_GetGamepadName(gamepad),
							m_Gamepads.back().playerID
						);
					}
				}
			}
		}

		SDL_free(gamepadIDs);
	}

	void InputManager::HandleMouseAction(SDL_Event const& event, Uint32 const evType, MouseInfo::ActionType const actType)
	{
		if (event.type == evType)
		{
			if (evType == SDL_EVENT_MOUSE_MOTION)
			{
				m_MouseDeltaX = event.motion.xrel;
				m_MouseDeltaY = event.motion.yrel;
			}
			else if (evType == SDL_EVENT_MOUSE_WHEEL)
			{
				float const scrollX{ event.wheel.x };
				float const scrollY{ event.wheel.y };

				m_MouseScrollX = scrollX;
				m_MouseScrollY = scrollY;
			}

			auto const& actions{ m_MappedMouseActions[static_cast<size_t>(actType)] };
			auto const it
			{ actions.find(
					evType == SDL_EVENT_MOUSE_WHEEL || evType == SDL_EVENT_MOUSE_MOTION || evType == SDL_EVENT_WINDOW_MOUSE_ENTER || evType == SDL_EVENT_WINDOW_MOUSE_LEAVE
					? 0
					: event.button.button)
			};

			if (it != end(actions))
			{
				for (auto const& action : it->second)
				{
					m_ExecutedActions[0].emplace(action);
				}
			}
		}
	}

	void InputManager::HandleMouseHeldAndMovement()
	{
		ME_PROFILE_FUNCTION()

		float x{ m_MouseX };
		float y{ m_MouseY };
		SDL_MouseButtonFlags const mouseButtonState{ SDL_GetMouseState(&x, &y) };

		auto const& actions{ m_MappedMouseActions[static_cast<size_t>(KeyInfo::ActionType::Held)] };
		auto handleMouseBtnHeld{ [&](int const mask, uint8_t const button)
			{
				if (mouseButtonState & mask)
				{
					auto it{ actions.find(button) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_ExecutedActions[0].emplace(action);
						}
					}
				}
			} };

		handleMouseBtnHeld(SDL_BUTTON_LMASK, SDL_BUTTON_LEFT);
		handleMouseBtnHeld(SDL_BUTTON_RMASK, SDL_BUTTON_RIGHT);
		handleMouseBtnHeld(SDL_BUTTON_X1MASK, SDL_BUTTON_X1);
		handleMouseBtnHeld(SDL_BUTTON_X2MASK, SDL_BUTTON_X2);
		handleMouseBtnHeld(SDL_BUTTON_MMASK, SDL_BUTTON_MIDDLE);
	}

	void InputManager::HandleKeyboardHeld()
	{
		ME_PROFILE_FUNCTION()

		auto const& actions{ m_MappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Held)] };
		int numKeys{ };
		bool const* keyState{ SDL_GetKeyboardState(&numKeys) };
		if (numKeys > 0 && keyState)
		{
			for (auto const& keys : actions)
			{
				SDL_Scancode scancode{ SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(keys.first), NULL) };
				if (keyState[scancode])
				{
					for (auto const& action : keys.second)
					{
						m_ExecutedActions[0].emplace(action);
					}
				}
			}
		}
	}

	void InputManager::HandleGamepadHeld()
	{
		ME_PROFILE_FUNCTION()

		for (auto& g : m_Gamepads)
		{
			auto const& actions{ m_MappedGamepadActions[g.playerID][static_cast<size_t>(GamepadInfo::ActionType::Held)] };
			for (auto const& buttons : actions)
			{
				SDL_GamepadButton const button{ static_cast<SDL_GamepadButton>(buttons.first) };
				if (SDL_GetGamepadButton(g.gamepad, button))
				{
					for (auto const& action : buttons.second)
					{
						m_ExecutedActions[g.playerID].emplace(action);
					}
				}
			}
		}
	}

	void InputManager::HandleGamepadAxisState()
	{
		// TODO need to handle held

		for (auto& g : m_Gamepads)
		{
			for (uint32_t axis{ 0 }; axis < SDL_GAMEPAD_AXIS_COUNT; ++axis)
			{
				float const raw{ static_cast<float>(SDL_GetGamepadAxis(g.gamepad, static_cast<SDL_GamepadAxis>(axis))) };
				float norm{ raw / 32767.0f };

				if (axis == SDL_GAMEPAD_AXIS_LEFTX
				or  axis == SDL_GAMEPAD_AXIS_LEFTY
				or  axis == SDL_GAMEPAD_AXIS_RIGHTX
				or  axis == SDL_GAMEPAD_AXIS_RIGHTY)
				{
					if (std::abs(norm) <= m_Deadzone)
					{
						norm = 0.f;
					}
					else
					{
						// normalise so the deadzone doesnt disrupt the -1;1 range
						float const sign{ (norm > 0.f) ? 1.f : -1.f };
						norm = sign * ((std::abs(norm) - m_Deadzone) / (1.f - m_Deadzone));
					}
				}

				if (norm != 0.f)
				{
					auto const& actions{ m_MappedGamepadActions[g.playerID][static_cast<size_t>(GamepadInfo::ActionType::AxisHeld)] };
					// held action
					auto const it{ actions.find(axis) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_ExecutedActions[g.playerID].emplace(action);
						}
					}
				}

				m_GamepadAxes[g.playerID].delta[axis] = norm - m_GamepadAxes[g.playerID].current[axis];
				m_GamepadAxes[g.playerID].current[axis] = norm;
			}
		}
	}

	void InputManager::ResetState()
	{
		for (auto& actions : m_ExecutedActions)
		{
			actions.clear();
		}

		m_MouseDeltaX = 0.f;
		m_MouseDeltaY = 0.f;
		m_MouseScrollX = 0.f;
		m_MouseScrollY = 0.f;

		for (auto& a : m_GamepadAxes)
		{
			for (auto& d : a.delta)
			{
				d = 0.f;
			}
		}

		for (size_t i = 0; i < m_Gamepads.size(); )
		{
			auto& p = m_Gamepads[i];
			if (p.markedForRemove)
			{
				ME_LOG_INFO(
					MauCor::LogCategory::Engine,
					"Disconnected controller: {} player ID: {}",
					SDL_GetGamepadName(p.gamepad),
					m_Gamepads[i].playerID
				);

				bool const result{ SDL_SetGamepadPlayerIndex(p.gamepad, -1) };
				ME_ENGINE_ASSERT(result);

				SDL_CloseGamepad(p.gamepad);
				m_AvailablePlayerIDs.emplace_back(p.playerID);
				std::swap(p, m_Gamepads.back());
				m_Gamepads.pop_back();

				// Don't increment i; we need to process the swapped-in item
				continue;
			}
			++i;
		}
	}

	bool InputManager::ProcessInput() noexcept
	{
		ME_PROFILE_FUNCTION()

		ResetState();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			// Quit event -> return false so the gameloop can stop
			if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			{
				return false;
			}

			// Mouse
			HandleMouseAction(event, SDL_EVENT_MOUSE_BUTTON_DOWN, MouseInfo::ActionType::Down);
			HandleMouseAction(event, SDL_EVENT_MOUSE_BUTTON_UP, MouseInfo::ActionType::Up);
			HandleMouseAction(event, SDL_EVENT_MOUSE_WHEEL, MouseInfo::ActionType::Scrolled);
			HandleMouseAction(event, SDL_EVENT_MOUSE_MOTION, MouseInfo::ActionType::Moved);
			HandleMouseAction(event, SDL_EVENT_WINDOW_MOUSE_ENTER, MouseInfo::ActionType::EnteredWindow);
			HandleMouseAction(event, SDL_EVENT_WINDOW_MOUSE_LEAVE, MouseInfo::ActionType::LeftWindow);

			// Keyboard
			//Down this frame
			if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
			{
				auto const& actions{ m_MappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Down)] };
				auto const it{ actions.find(static_cast<uint32_t>(event.key.key)) };
				if (it != end(actions))
				{
					for (auto const& action : it->second)
					{
						m_ExecutedActions[0].emplace(action);
					}
				}
			}
			//Up this frame
			else if (event.type == SDL_EVENT_KEY_UP)
			{
				auto const& actions{ m_MappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Up)] };
				auto it{ actions.find(static_cast<uint32_t>(event.key.key)) };
				if (it != end(actions))
				{
					for (auto const& action : it->second)
					{
						m_ExecutedActions[0].emplace(action);
					}
				}
			}

			// Gamepad
			if (event.type == SDL_EVENT_GAMEPAD_ADDED)
			{
				if (not m_AvailablePlayerIDs.empty())
				{
					auto const id{ event.gdevice.which };

					bool alreadyExists{ false };
					for (auto& p : m_Gamepads)
					{
						auto const connectedId{ SDL_GetGamepadID(p.gamepad) };

						if (connectedId == id)
						{
							alreadyExists = true;
							break;
						}
					}

					if (SDL_IsGamepad(id) and not alreadyExists)
					{
						SDL_Gamepad* gamepad{ SDL_OpenGamepad(id) };
						if (gamepad)
						{
							m_Gamepads.emplace_back(gamepad, m_AvailablePlayerIDs.back());
							m_AvailablePlayerIDs.pop_back();

							bool const result{ SDL_SetGamepadPlayerIndex(gamepad, static_cast<int>(m_Gamepads.back().playerID)) };
							ME_ENGINE_ASSERT(result);

							ME_LOG_INFO(
								MauCor::LogCategory::Engine,
								"Connected controller: {} player ID: {}",
								SDL_GetGamepadName(gamepad),
								m_Gamepads.back().playerID
							);
						}
					}
				}
			}
			else if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
			{
				auto const removedId{ event.gdevice.which };

				for (uint32_t i {0}; i < m_Gamepads.size(); ++i)
				{
					auto& p{ m_Gamepads[i] };

					auto const id{ SDL_GetGamepadID(p.gamepad) };
					if (id == removedId)
					{
						p.markedForRemove = true;
						break;
					}
				}
			}

			if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN && !event.key.repeat)
			{
				auto const id{ event.gdevice.which };
				auto const playerID{ SDL_GetGamepadPlayerIndexForID(id) };

				ME_ENGINE_ASSERT(playerID != -1);
				if (playerID != -1)
				{
					ME_ENGINE_ASSERT(playerID <= 3 && playerID >= 0);
					auto const& actions{ m_MappedGamepadActions[playerID][static_cast<size_t>(GamepadInfo::ActionType::Down)] };
					auto const it{ actions.find(static_cast<uint32_t>(event.gbutton.button)) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_ExecutedActions[playerID].emplace(action);
						}
					}
				}
			}
			else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP)
			{
				auto const id{ event.gdevice.which };
				auto const playerID{ SDL_GetGamepadPlayerIndexForID(id) };

				ME_ENGINE_ASSERT(playerID != -1);
				if (playerID != -1)
				{
					ME_ENGINE_ASSERT(playerID <= 3 && playerID >= 0);
					auto const& actions{ m_MappedGamepadActions[playerID][static_cast<size_t>(GamepadInfo::ActionType::Up)] };
					auto const it{ actions.find(static_cast<uint32_t>(event.gbutton.button)) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_ExecutedActions[playerID].emplace(action);
						}
					}
				}
			}
			else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION)
			{
				auto const id{ event.gdevice.which };
				auto const playerID{ SDL_GetGamepadPlayerIndexForID(id) };

				ME_ENGINE_ASSERT(playerID != -1);
				if (playerID != -1)
				{
					ME_ENGINE_ASSERT(playerID <= 3 && playerID >= 0);
					auto const& actions{ m_MappedGamepadActions[playerID][static_cast<size_t>(GamepadInfo::ActionType::AxisMoved)] };
					auto const it{ actions.find(static_cast<uint32_t>(event.gaxis.axis)) };
					
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX
								or event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY
								or event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHTX
								or event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHTY)
							{
								float const raw{ static_cast<float>(event.gaxis.value)};
								float const norm{ raw / 32767.0f };

								if (std::abs(norm) >= m_Deadzone)
								{
									m_ExecutedActions[playerID].emplace(action);
								}
							}
						}
					}
				}
			}
		}

		HandleMouseHeldAndMovement();
		HandleKeyboardHeld();
		HandleGamepadHeld();
		HandleGamepadAxisState();

		return true;
	}

	void InputManager::Destroy()
	{
		for (auto& p : m_Gamepads)
		{
			SDL_CloseGamepad(p.gamepad);
			m_AvailablePlayerIDs.emplace_back(p.playerID);
		}

		m_Gamepads.clear();
	}

	void InputManager::BindAction(std::string const& actionName, KeyInfo const& keyInfo) noexcept
	{
		auto& actionTypeVec{ m_MappedKeyboardActions[static_cast<size_t>(keyInfo.type)] };
		actionTypeVec[static_cast<uint32_t>(keyInfo.key)].emplace_back(actionName);

		m_ActionToKeyboardKey[actionName].emplace_back(static_cast<uint32_t>(keyInfo.key));
	}

	void InputManager::BindAction(std::string const& actionName, MouseInfo const& mouseInfo) noexcept
	{
		auto& actionTypeVec{ m_MappedMouseActions[static_cast<size_t>(mouseInfo.type)] };
		actionTypeVec[mouseInfo.button].emplace_back(actionName);

		m_ActionToMouseButton[actionName].emplace_back(mouseInfo.button);
	}

	void InputManager::BindAction(std::string const& actionName, GamepadInfo const& gamepadInfo, uint32_t playerID) noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "Player ID out of bounds");

		uint32_t btnAxis{ 0 };
		if (gamepadInfo.type == GamepadInfo::ActionType::AxisHeld or gamepadInfo.type == GamepadInfo::ActionType::AxisMoved)
		{
			btnAxis = gamepadInfo.input.axis;
		}
		else
		{
			btnAxis = gamepadInfo.input.button;
		}

		auto& actionVecTypeVec{ m_MappedGamepadActions[playerID][static_cast<size_t>(gamepadInfo.type)] };
		actionVecTypeVec[btnAxis].emplace_back(actionName);

		m_ActionToGamepad[playerID][actionName].emplace_back(btnAxis);
	}

	void InputManager::UnBindAction(std::string const& actionName, uint32_t playerID) noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "PlayerID out of bounds");

		// gamepad
		{
			auto const it{ m_ActionToGamepad[playerID].find(actionName) };
			if (it != end(m_ActionToGamepad[playerID]))
			{
				for (auto&& b : it->second)
				{
					for (auto& mappedGamepad : m_MappedGamepadActions[playerID])
					{
						// need to find the string and erase then
						std::erase_if(mappedGamepad[b],
							[&actionName](std::string const& a)
							{
								return a == actionName;
							});
					}
				}
				m_ActionToGamepad[playerID].erase(it);
			}
		}

		// Keyboard is always playerID 0
		if (0 == playerID)
		{
			{
				auto const it{ m_ActionToMouseButton.find(actionName) };
				if (it != end(m_ActionToMouseButton))
				{
					for (auto&& b : it->second)
					{
						for (auto& mappedMouse : m_MappedMouseActions)
						{
							// need to find the string and erase then
							std::erase_if(mappedMouse[b],
								[&actionName](std::string const& a)
								{
									return a == actionName;
								});
						}
					}
					m_ActionToMouseButton.erase(it);
				}
			}

			{
				auto const it{ m_ActionToKeyboardKey.find(actionName) };
				if (it != end(m_ActionToKeyboardKey))
				{
					for (auto&& b : it->second)
					{
						for (auto& mappedKeyboard : m_MappedKeyboardActions)
						{
							std::erase_if(mappedKeyboard[b],
								[&actionName](std::string const& a)
								{
									return a == actionName;
								});
						}
					}
					m_ActionToKeyboardKey.erase(it);
				}
			}
		}


	}

	void InputManager::UnBindAllActions(uint32_t playerID) noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "PlayerID out of bounds");

		if (0 == playerID)
		{
			for (auto& m : m_MappedKeyboardActions)
			{
				m.clear();
			}
			m_ActionToKeyboardKey.clear();

			for (auto& m : m_MappedMouseActions)
			{
				m.clear();
			}
			m_ActionToMouseButton.clear();
		}

		for (auto& m : m_MappedGamepadActions[playerID])
		{
			m.clear();
		}
		m_ActionToGamepad[playerID].clear();
	}

	void InputManager::UnBindAllActions(KeyInfo const& keyInfo) noexcept
	{
		auto const type{ static_cast<uint32_t>(keyInfo.type) };

		auto const it{ m_MappedKeyboardActions[type].find(keyInfo.key) };

		if (it == end(m_MappedKeyboardActions[type]))
		{
			return;
		}

		// erase in m_ActionToKeyboardKey
		for (auto&& a : it->second)
		{
			std::erase_if(m_ActionToKeyboardKey[a],
				[&keyInfo](uint32_t k)
				{
					return k == keyInfo.key;
				});
		}

		// erase in mapped
		m_MappedKeyboardActions[type].erase(it);
	}

	void InputManager::UnBindAllActions(MouseInfo const& mouseInfo) noexcept
	{
		auto const type{ static_cast<uint32_t>(mouseInfo.type) };

		auto const it{ m_MappedMouseActions[type].find(mouseInfo.button) };

		if (it == end(m_MappedMouseActions[type]))
		{
			return;
		}

		// erase in m_ActionToMouseButton
		for (auto&& a : it->second)
		{
			std::erase_if(m_ActionToMouseButton[a],
				[&mouseInfo](uint8_t b)
				{
					return b == mouseInfo.button;
				});
		}

		// erase in mapped
		m_MappedMouseActions[type].erase(it);
	}

	bool InputManager::HasControllerForPlayerID(uint32_t playerID) const noexcept
	{
		return std::ranges::any_of(
			m_Gamepads,
			[playerID](const auto& entry) { return entry.playerID == playerID; });
	}

	uint32_t InputManager::NumConnectedControllers() const noexcept
	{
		return static_cast<uint32_t>(m_Gamepads.size());
	}

	bool InputManager::IsActionExecuted(std::string const& actionName, uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "Engine only supports 4 players");
		ME_ENGINE_ASSERT(playerID < m_ExecutedActions.size(), "No player created for requested playerID");
		return m_ExecutedActions[playerID].contains(actionName);
	}

	std::pair<float, float> InputManager::GetLeftJoystick(uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "Engine only supports 4 players");

		const auto& axes{ m_GamepadAxes[playerID].current };
		return { axes[SDL_GAMEPAD_AXIS_LEFTX], axes[SDL_GAMEPAD_AXIS_LEFTY] };
	}

	std::pair<float, float> InputManager::GetDeltaLeftJoystick(uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "Engine only supports 4 players");

		const auto& deltas{ m_GamepadAxes[playerID].delta };
		return { deltas[SDL_GAMEPAD_AXIS_LEFTX], deltas[SDL_GAMEPAD_AXIS_LEFTY] };
	}

	std::pair<float, float> InputManager::GetRightJoystick(uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "Engine only supports 4 players");

		const auto& axes{ m_GamepadAxes[playerID].current };
		return { axes[SDL_GAMEPAD_AXIS_RIGHTX], axes[SDL_GAMEPAD_AXIS_RIGHTY] };
	}

	std::pair<float, float> InputManager::GetDeltaRightJoystick(uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "Engine only supports 4 players");

		const auto& deltas{ m_GamepadAxes[playerID].delta };
		return { deltas[SDL_GAMEPAD_AXIS_RIGHTX], deltas[SDL_GAMEPAD_AXIS_RIGHTY] };
	}

	float InputManager::GetLeftTrigger(uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "Engine only supports 4 players");

		return m_GamepadAxes[playerID].current[SDL_GAMEPAD_AXIS_LEFT_TRIGGER];
	}

	float InputManager::GetDeltaLeftTrigger(uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "Engine only supports 4 players");

		return m_GamepadAxes[playerID].delta[SDL_GAMEPAD_AXIS_LEFT_TRIGGER];
	}

	float InputManager::GetRightTrigger(uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "Engine only supports 4 players");

		return m_GamepadAxes[playerID].current[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER];
	}

	float InputManager::GetDeltaRightTrigger(uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "Engine only supports 4 players");

		return m_GamepadAxes[playerID].delta[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER];
	}

	void InputManager::SetDeadzone(float newDeadzone) noexcept
	{
		ME_ENGINE_ASSERT(newDeadzone >= 0.f && newDeadzone < 1.F);
		m_Deadzone = newDeadzone;
	}

	void InputManager::Clear() noexcept
	{
		m_MappedKeyboardActions.clear();
		m_ActionToKeyboardKey.clear();

		m_MappedMouseActions.clear();
		m_ActionToMouseButton.clear();

		for (auto& actions : m_MappedGamepadActions)
		{
			actions.clear();
		}
		for (auto& actions : m_ActionToGamepad)
		{
			actions.clear();
		}
		for (auto& actions : m_ExecutedActions)
		{
			actions.clear();
		}

		ResetState();
	}

}
