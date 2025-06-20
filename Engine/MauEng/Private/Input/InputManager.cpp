#include "EnginePCH.h"
#include "Input/InputManager.h"

#include <SDL3/SDL.h>

namespace MauEng
{
	InputManager::InputManager()
	{
		m_KeyboardContexts["DEFAULT"] = {};
		m_KeyboardContexts["DEFAULT"].mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
		m_KeyboardContexts["DEFAULT"].mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));
		m_ActiveKeyboardMouseContexts.resize(4);
		for (auto& a : m_ActiveKeyboardMouseContexts)
		{
			a = "DEFAULT";
		}

		m_GamepadContexts["DEFAULT"] = {};
		m_GamepadContexts["DEFAULT"].mappedGamepadActions.resize(static_cast<size_t>(GamepadInfo::ActionType::COUNT));
		m_ActiveGamepadMappingContexts.resize(4);
		for (auto& a : m_ActiveGamepadMappingContexts)
		{
			a = "DEFAULT";
		}

		if constexpr (SKIP_CONTROLLER_INPUT_PLAYER_ID_0)
		{
			m_AvailablePlayerIDs.pop_back();
		}

		m_ExecutedActions.resize(4);

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

			for (uint32_t i{ 0 }; i < m_ActiveKeyboardMouseContexts.size(); ++i)
			{
				auto& p{ m_ActiveKeyboardMouseContexts[i] };
				auto const& actions{ m_KeyboardContexts[p].mappedMouseActions[static_cast<size_t>(actType)] };
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
						m_ExecutedActions[i].emplace(action);
					}
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


		auto handleMouseBtnHeld{ [&](int const mask, uint8_t const button)
			{
				if (mouseButtonState & mask)
				{
					for (uint32_t i{ 0 }; i < m_ActiveKeyboardMouseContexts.size(); ++i)
					{
						auto& p{ m_ActiveKeyboardMouseContexts[i] };
						auto const& actions{ m_KeyboardContexts[p].mappedMouseActions[static_cast<size_t>(KeyInfo::ActionType::Held)] };
						auto const it{ actions.find(button) };
						if (it != end(actions))
						{
							for (auto const& action : it->second)
							{
								m_ExecutedActions[i].emplace(action);
							}
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

		int numKeys{ };
		bool const* keyState{ SDL_GetKeyboardState(&numKeys) };
		if (numKeys > 0 && keyState)
		{
			for (uint32_t i{ 0 }; i < m_ActiveKeyboardMouseContexts.size(); ++i)
			{
				auto& p{ m_ActiveKeyboardMouseContexts[i] };
				auto const& actions{ m_KeyboardContexts[p].mappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Held)] };
				for (auto const& keys : actions)
				{
					SDL_Scancode scancode{ SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(keys.first), NULL) };
					if (keyState[scancode])
					{
						for (auto const& action : keys.second)
						{
							m_ExecutedActions[i].emplace(action);
						}
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
			auto const& actions{ m_GamepadContexts[m_ActiveGamepadMappingContexts[g.playerID]].mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::Held)] };
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
		ME_PROFILE_FUNCTION()

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
					if (std::abs(norm) <= m_JoystickDeadzone)
					{
						norm = 0.f;
					}
					else
					{
						// normalise so the deadzone doesnt disrupt the -1;1 range
						float const sign{ (norm > 0.f) ? 1.f : -1.f };
						norm = sign * ((std::abs(norm) - m_JoystickDeadzone) / (1.f - m_JoystickDeadzone));
					}
				}
				else if ( axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER 
					   or axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
				{
					if (std::abs(norm) <= m_TriggerDeadzone)
					{
						norm = 0.f;
					}
					else
					{
						// normalise so the deadzone doesnt disrupt the -1;1 range
						float const sign{ (norm > 0.f) ? 1.f : -1.f };
						norm = sign * ((std::abs(norm) - m_TriggerDeadzone) / (1.f - m_TriggerDeadzone));
					}
				}
				
				if (norm != 0.f)
				{
					if (not m_GamepadAxes[g.playerID].held[axis])
					{
						// broadaast start held
						auto const& actions{ m_GamepadContexts[m_ActiveGamepadMappingContexts[g.playerID]].mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::AxisStartHeld)] };
						auto const it{ actions.find(axis) };
						if (it != end(actions))
						{
							for (auto const& action : it->second)
							{
								m_ExecutedActions[g.playerID].emplace(action);
							}
						}
					}

					m_GamepadAxes[g.playerID].held[axis] = true;

					auto const& actions{ m_GamepadContexts[m_ActiveGamepadMappingContexts[g.playerID]].mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::AxisHeld)] };
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
				else
				{
					if (m_GamepadAxes[g.playerID].held[axis])
					{
						// broadaast start held
						auto const& actions{ m_GamepadContexts[m_ActiveGamepadMappingContexts[g.playerID]].mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::AxisReleased)] };
						auto const it{ actions.find(axis) };
						if (it != end(actions))
						{
							for (auto const& action : it->second)
							{
								m_ExecutedActions[g.playerID].emplace(action);
							}
						}
					}
					m_GamepadAxes[g.playerID].held[axis] = false;
				}

				m_GamepadAxes[g.playerID].delta[axis] = norm - m_GamepadAxes[g.playerID].current[axis];
				m_GamepadAxes[g.playerID].current[axis] = norm;
			}
		}
	}

	void InputManager::ResetState()
	{
		ME_PROFILE_FUNCTION()

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
				for (uint32_t i{ 0 }; i < m_ActiveKeyboardMouseContexts.size(); ++i)
				{
					auto& p{ m_ActiveKeyboardMouseContexts[i] };
					auto const& actions{ m_KeyboardContexts[p].mappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Down)] };
					auto const it{ actions.find(static_cast<uint32_t>(event.key.key)) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_ExecutedActions[i].emplace(action);
						}
					}
				}
			}
			//Up this frame
			else if (event.type == SDL_EVENT_KEY_UP)
			{
				for (uint32_t i{ 0 }; i < m_ActiveKeyboardMouseContexts.size(); ++i)
				{
					auto& p{ m_ActiveKeyboardMouseContexts[i] };
					auto const& actions{ m_KeyboardContexts[p].mappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Up)] };
					auto it{ actions.find(static_cast<uint32_t>(event.key.key)) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_ExecutedActions[i].emplace(action);
						}
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
					auto const& actions{ m_GamepadContexts[m_ActiveGamepadMappingContexts[playerID]].mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::Down)] };
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
					auto const& actions{ m_GamepadContexts[m_ActiveGamepadMappingContexts[playerID]].mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::Up)] };
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
					auto const& actions{ m_GamepadContexts[m_ActiveGamepadMappingContexts[playerID]].mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::AxisMoved)] };
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

								if (std::abs(norm) >= m_JoystickDeadzone)
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

	void InputManager::SetMappingContext(std::string const& mappingContext, uint32_t playerID) noexcept
	{
		SetKeyboardMappingContext(mappingContext, playerID);
		SetGamepadMappingContext(mappingContext, playerID);
	}

	void InputManager::SetKeyboardMappingContext(std::string const& mappingContext, uint32_t playerID) noexcept
	{
		{
			auto const it{ m_KeyboardContexts.find(mappingContext) };
			if (it == end(m_KeyboardContexts))
			{
				ME_LOG_WARN(MauCor::LogCategory::Engine, "Creating new mapping context in SetMappingContext; was not created yet");
				m_KeyboardContexts[mappingContext] = {};
				m_KeyboardContexts[mappingContext].mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
				m_KeyboardContexts[mappingContext].mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));
			}
			m_ActiveKeyboardMouseContexts[playerID] = mappingContext;
		}
	}

	void InputManager::SetGamepadMappingContext(std::string const& mappingContext, uint32_t playerID) noexcept
	{
		{
			auto const it{ m_GamepadContexts.find(mappingContext) };
			if (it == end(m_GamepadContexts))
			{
				ME_LOG_WARN(MauCor::LogCategory::Engine, "Creating new mapping context in SetMappingContext; was not created yet");
				m_GamepadContexts[mappingContext] = {};
				m_GamepadContexts[mappingContext].mappedGamepadActions.resize(static_cast<size_t>(GamepadInfo::ActionType::COUNT));
			}
			m_ActiveGamepadMappingContexts[playerID] = mappingContext;
		}
	}

	std::string const& InputManager::GetKeyboardMappingContext(uint32_t playerID) const noexcept
	{
		ME_ASSERT(playerID < m_ActiveKeyboardMouseContexts.size());
		return m_ActiveKeyboardMouseContexts[playerID];
	}

	std::string const& InputManager::GetGamepadMappingContext(uint32_t playerID) const noexcept
	{
		ME_ASSERT(playerID < m_ActiveGamepadMappingContexts.size());
		return m_ActiveGamepadMappingContexts[playerID];
	}

	void InputManager::BindAction(std::string const& actionName, KeyInfo const& keyInfo, std::string const& mappingContext) noexcept
	{
		//keyboard
		auto const it{ m_KeyboardContexts.find(mappingContext) };
		if (it == end(m_KeyboardContexts))
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Creating new mapping context in SetMappingContext; was not created yet");
			m_KeyboardContexts[mappingContext] = {};
			m_KeyboardContexts[mappingContext].mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
			m_KeyboardContexts[mappingContext].mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));
		}

		auto& actionTypeVec{ m_KeyboardContexts[mappingContext].mappedKeyboardActions[static_cast<size_t>(keyInfo.type)] };
		actionTypeVec[static_cast<uint32_t>(keyInfo.key)].emplace_back(actionName);

		m_KeyboardContexts[mappingContext].actionToKeyboardKey[actionName].emplace_back(static_cast<uint32_t>(keyInfo.key));
	}

	void InputManager::BindAction(std::string const& actionName, MouseInfo const& mouseInfo, std::string const& mappingContext) noexcept
	{
		//keyboard
		auto const it{ m_KeyboardContexts.find(mappingContext) };
		if (it == end(m_KeyboardContexts))
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Creating new mapping context in SetMappingContext; was not created yet");
			m_KeyboardContexts[mappingContext] = {};
			m_KeyboardContexts[mappingContext].mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
			m_KeyboardContexts[mappingContext].mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));
		}

		auto& actionTypeVec{ m_KeyboardContexts[mappingContext].mappedMouseActions[static_cast<size_t>(mouseInfo.type)] };
		actionTypeVec[mouseInfo.button].emplace_back(actionName);

		m_KeyboardContexts[mappingContext].actionToMouseButton[actionName].emplace_back(mouseInfo.button);
	}

	void InputManager::BindAction(std::string const& actionName, GamepadInfo const& gamepadInfo, std::string const& mappingContext) noexcept
	{
		uint32_t btnAxis{ 0 };
		if (gamepadInfo.type == GamepadInfo::ActionType::AxisHeld 
		or  gamepadInfo.type == GamepadInfo::ActionType::AxisMoved
		or  gamepadInfo.type == GamepadInfo::ActionType::AxisReleased
		or  gamepadInfo.type == GamepadInfo::ActionType::AxisStartHeld)
		{
			btnAxis = gamepadInfo.input.axis;
		}
		else
		{
			btnAxis = gamepadInfo.input.button;
		}

		auto const it{ m_GamepadContexts.find(mappingContext) };
		if (it == end(m_GamepadContexts))
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Creating new mapping context in SetMappingContext; was not created yet");
			m_GamepadContexts[mappingContext] = {};
			m_GamepadContexts[mappingContext].mappedGamepadActions.resize(static_cast<size_t>(GamepadInfo::ActionType::COUNT));
		}

		auto& actionVecTypeVec{ m_GamepadContexts[mappingContext].mappedGamepadActions[static_cast<size_t>(gamepadInfo.type)] };
		actionVecTypeVec[btnAxis].emplace_back(actionName);

		m_GamepadContexts[mappingContext].actionToGamepad[actionName].emplace_back(btnAxis);
	}

	void InputManager::UnBindAction(std::string const& actionName, std::string const& mappingContext) noexcept
	{
		// gamepad
		{
			if (m_GamepadContexts.contains(mappingContext))
			{
				auto const it{ m_GamepadContexts[mappingContext].actionToGamepad.find(actionName) };
				if (it != end(m_GamepadContexts[mappingContext].actionToGamepad))
				{
					for (auto&& b : it->second)
					{
						for (auto& mappedGamepad : m_GamepadContexts[mappingContext].mappedGamepadActions)
						{
							// need to find the string and erase then
							std::erase_if(mappedGamepad[b],
								[&actionName](std::string const& a)
								{
									return a == actionName;
								});
						}
					}

					m_GamepadContexts[mappingContext].actionToGamepad.erase(it);
				}
			}
			else
			{
				ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to unbind action for mapping context that doesnt exist gamepad");
			}
		}

		{
			if (m_KeyboardContexts.contains(mappingContext))
			{
				auto const it{ m_KeyboardContexts[mappingContext].actionToMouseButton.find(actionName) };
				if (it != end(m_KeyboardContexts[mappingContext].actionToMouseButton))
				{
					for (auto&& b : it->second)
					{
						for (auto& mappedMouse : m_KeyboardContexts[mappingContext].mappedMouseActions)
						{
							// need to find the string and erase then
							std::erase_if(mappedMouse[b],
								[&actionName](std::string const& a)
								{
									return a == actionName;
								});
						}
					}
					m_KeyboardContexts[mappingContext].actionToMouseButton.erase(it);
				}
			}
			else
			{
				ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to unbind action for mapping context that doesnt exist (keyboard & mouse)");
			}
		}

		{
			if (m_KeyboardContexts.contains(mappingContext))
			{
				auto const it{ m_KeyboardContexts[mappingContext].actionToKeyboardKey.find(actionName) };
				if (it != end(m_KeyboardContexts[mappingContext].actionToKeyboardKey))
				{
					for (auto&& b : it->second)
					{
						for (auto& mappedKeyboard : m_KeyboardContexts[mappingContext].mappedKeyboardActions)
						{
							std::erase_if(mappedKeyboard[b],
								[&actionName](std::string const& a)
								{
									return a == actionName;
								});
						}
					}
					m_KeyboardContexts[mappingContext].actionToKeyboardKey.erase(it);
				}
			}
			else
			{
				ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to unbind action for mapping context that doesnt exist (keyboard & mouse)");
			}
		}
	}

	void InputManager::UnBindAllActions(KeyInfo const& keyInfo, std::string const& mappingContext) noexcept
	{
		auto const type{ static_cast<uint32_t>(keyInfo.type) };

		{
			auto const it{ m_KeyboardContexts.find(mappingContext) };
			if (it == end(m_KeyboardContexts))
			{
				ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to unbind all actions (for a given keyinfo) for a mapping context that does not exist");
				return;
			}
		}

		auto const it{ m_KeyboardContexts[mappingContext].mappedKeyboardActions[type].find(keyInfo.key) };
		if (it == end(m_KeyboardContexts[mappingContext].mappedKeyboardActions[type]))
		{
			return;
		}

		// erase in m_ActionToKeyboardKey
		for (auto&& a : it->second)
		{
			std::erase_if(m_KeyboardContexts[mappingContext].actionToKeyboardKey[a],
				[&keyInfo](uint32_t k)
				{
					return k == keyInfo.key;
				});
		}

		// erase in mapped
		m_KeyboardContexts[mappingContext].mappedKeyboardActions[type].erase(it);
	}

	void InputManager::UnBindAllActions(GamepadInfo const& gamepadInfo, std::string const& mappingContext) noexcept
	{
		auto const type{ static_cast<uint32_t>(gamepadInfo.type) };

		{
			auto const it{ m_GamepadContexts.find(mappingContext) };
			if (it == end(m_GamepadContexts))
			{
				ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to unbind all actions (for a given gamepadinfo) for a mapping context that does not exist");
				return;
			}
		}

		auto& mappedActions{ m_GamepadContexts[mappingContext].mappedGamepadActions[type] };
		uint32_t btnAxis{ 0 };
		if (gamepadInfo.type == GamepadInfo::ActionType::AxisHeld
			or gamepadInfo.type == GamepadInfo::ActionType::AxisMoved
			or gamepadInfo.type == GamepadInfo::ActionType::AxisReleased
			or gamepadInfo.type == GamepadInfo::ActionType::AxisStartHeld)
		{
			btnAxis = gamepadInfo.input.axis;
		}
		else
		{
			btnAxis = gamepadInfo.input.button;
		}

		auto const it{ mappedActions.find(btnAxis) };

		if (it == end(mappedActions))
		{
			return;
		}

		// erase in m_ActionToGamepad
		for (auto&& a : it->second)
		{
			std::erase_if(m_GamepadContexts[mappingContext].actionToGamepad[a],
				[btnAxis](uint8_t b)
				{
					return b == btnAxis;
				});
		}

		// erase in mapped
		mappedActions.erase(it);
	}

	void InputManager::UnBindAllActions(std::string const& mappingContext) noexcept
	{
		{
			auto const it{ m_KeyboardContexts.find(mappingContext) };
			if (it != end(m_KeyboardContexts))
			{
				for (auto& v : it->second.mappedKeyboardActions)
				{
					v.clear();
				}
				for (auto& v : it->second.mappedMouseActions)
				{
					v.clear();
				}

				it->second.actionToKeyboardKey.clear();
				it->second.actionToMouseButton.clear();
			}
		}
		{
			auto const it{ m_GamepadContexts.find(mappingContext) };
			if (it != end(m_GamepadContexts))
			{
				for (auto& v : it->second.mappedGamepadActions)
				{
					v.clear();
				}

				it->second.actionToGamepad.clear();
			}
		}
	}

	void InputManager::EraseMappingContext(std::string const& mappingContext,
		std::string const& newMappingContextIfErasedIsActive) noexcept
	{
		EraseKeyboardMappingContext(mappingContext, newMappingContextIfErasedIsActive);
		EraseGamepadMappingContext(mappingContext, newMappingContextIfErasedIsActive);
	}

	void InputManager::EraseKeyboardMappingContext(std::string const& mappingContext,
		std::string const& newMappingContextIfErasedIsActive) noexcept
	{
		auto const it{ m_KeyboardContexts.find(mappingContext) };
		if (it != end(m_KeyboardContexts))
		{
			m_KeyboardContexts.erase(it);
		}

		if (!m_KeyboardContexts.contains(newMappingContextIfErasedIsActive))
		{
			m_KeyboardContexts[newMappingContextIfErasedIsActive] = {};
			m_KeyboardContexts[newMappingContextIfErasedIsActive].mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
			m_KeyboardContexts[newMappingContextIfErasedIsActive].mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));
		}

		for (auto& c : m_ActiveKeyboardMouseContexts)
		{
			if (c == mappingContext)
			{
				c = newMappingContextIfErasedIsActive;
			}
		}
	}

	void InputManager::EraseGamepadMappingContext(std::string const& mappingContext,
		std::string const& newMappingContextIfErasedIsActive) noexcept
	{
		auto const it{ m_GamepadContexts.find(mappingContext) };
		if (it != end(m_GamepadContexts))
		{
			m_GamepadContexts.erase(it);
		}

		if (!m_GamepadContexts.contains(newMappingContextIfErasedIsActive))
		{
			m_GamepadContexts[newMappingContextIfErasedIsActive] = {};
			m_GamepadContexts[newMappingContextIfErasedIsActive].mappedGamepadActions.resize(static_cast<size_t>(GamepadInfo::ActionType::COUNT));
		}

		for (auto& c : m_ActiveGamepadMappingContexts)
		{
			if (c == mappingContext)
			{
				c = newMappingContextIfErasedIsActive;
			}
		}
	}

	void InputManager::UnBindAllActions(MouseInfo const& mouseInfo, std::string const& mappingContext) noexcept
	{
		auto const type{ static_cast<uint32_t>(mouseInfo.type) };

		{
			auto const it{ m_KeyboardContexts.find(mappingContext) };
			if (it == end(m_KeyboardContexts))
			{
				ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to unbind all actions (for a given mouseinfo) for a mapping context that does not exist");
				return;
			}
		}

		auto const it{ m_KeyboardContexts[mappingContext].mappedMouseActions[type].find(mouseInfo.button) };
		if (it == end(m_KeyboardContexts[mappingContext].mappedMouseActions[type]))
		{
			return;
		}

		// erase in m_ActionToMouseButton
		for (auto&& a : it->second)
		{
			std::erase_if(m_KeyboardContexts[mappingContext].actionToMouseButton[a],
				[&mouseInfo](uint8_t b)
				{
					return b == mouseInfo.button;
				});
		}

		// erase in mapped
		m_KeyboardContexts[mappingContext].mappedMouseActions[type].erase(it);
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

	void InputManager::SetJoystickDeadzone(float newDeadzone) noexcept
	{
		ME_ENGINE_ASSERT(newDeadzone >= 0.f && newDeadzone < 1.F);
		m_JoystickDeadzone = newDeadzone;
	}

	void InputManager::SetTriggerDeadzone(float newDeadzone) noexcept
	{
		ME_ENGINE_ASSERT(newDeadzone >= 0.f && newDeadzone < 1.F);
		m_TriggerDeadzone = newDeadzone;
	}

	void InputManager::Clear() noexcept
	{
		for (auto& actions : m_ExecutedActions)
		{
			actions.clear();
		}

		m_GamepadContexts.clear();
		m_KeyboardContexts.clear();

		m_KeyboardContexts["DEFAULT"] = {};
		m_KeyboardContexts["DEFAULT"].mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
		m_KeyboardContexts["DEFAULT"].mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));
		m_ActiveKeyboardMouseContexts.resize(4);
		for (auto& a : m_ActiveKeyboardMouseContexts)
		{
			a = "DEFAULT";
		}

		m_GamepadContexts["DEFAULT"] = {};
		m_GamepadContexts["DEFAULT"].mappedGamepadActions.resize(static_cast<size_t>(GamepadInfo::ActionType::COUNT));
		m_ActiveGamepadMappingContexts.resize(4);
		for (auto& a : m_ActiveGamepadMappingContexts)
		{
			a = "DEFAULT";
		}

		ResetState();
	}

}
