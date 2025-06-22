#include "EnginePCH.h"
#include "Input/InputManager.h"

#include <SDL3/SDL.h>

namespace MauEng
{
	InputManager::InputManager()
	{
		m_InputDelegateImmediate += MauCor::Bind<InputEvent>(
			[this](InputEvent const& e) 
			{ m_ExecutedActions[e.playerID].emplace(e.action); });

		{
			auto const insertedIt{ m_KeyboardContexts.emplace("DEFAULT", KeyboardMouseMappingContext{} ) };
			insertedIt.first->second.mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
			insertedIt.first->second.mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));
			m_ActiveKeyboardMouseContexts.resize(4);
			for (auto& a : m_ActiveKeyboardMouseContexts)
			{
				a = "DEFAULT";
			}
		}

		{
			auto const insertedIt{ m_GamepadContexts.emplace("DEFAULT", GamepadMappingContext{}) };
			insertedIt.first->second.mappedGamepadActions.resize(static_cast<size_t>(GamepadInfo::ActionType::COUNT));
			m_ActiveGamepadMappingContexts.resize(4);
			for (auto& a : m_ActiveGamepadMappingContexts)
			{
				a = "DEFAULT";
			}

		}

		if constexpr (SKIP_CONTROLLER_INPUT_PLAYER_ID_0)
		{
			m_AvailablePlayerIDs_Gamepads.pop_back();
		}

		m_ExecutedActions.resize(4);

		int numGamepads{ 0 };
		SDL_JoystickID* gamepadIDs{ SDL_GetGamepads(&numGamepads) };
		if (gamepadIDs)
		{
			for (int i{ 0 }; i < numGamepads; ++i)
			{
				if (m_AvailablePlayerIDs_Gamepads.empty())
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
						m_Gamepads.emplace_back(gamepad, m_AvailablePlayerIDs_Gamepads.back());
						m_AvailablePlayerIDs_Gamepads.pop_back();

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

	void InputManager::Init() noexcept
	{
		CreatePlayer<Player>();
	}

	void InputManager::HandleMouseAction(SDL_Event const& event, Uint32 const evType, MouseInfo::ActionType const actType, std::array<std::unordered_map<std::string, KeyboardMouseMappingContext>::iterator, 4> const& contexts)
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
				auto const& actions{ contexts[i]->second.mappedMouseActions[static_cast<size_t>(actType)]};
				if (evType == SDL_EVENT_MOUSE_WHEEL || evType == SDL_EVENT_MOUSE_MOTION || evType == SDL_EVENT_WINDOW_MOUSE_ENTER || evType == SDL_EVENT_WINDOW_MOUSE_LEAVE)
				{
					for (auto const& buttonActions : actions)
					{
						for (auto const& a : buttonActions.second)
						{
							m_InputDelegateImmediate < InputEvent{ i, a };
							m_InputDelegateDelayed[i] << InputEvent{ i, a };
						}
					}
				}
				else
				{
					auto const it{ actions.find(event.button.button) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_InputDelegateImmediate < InputEvent{ i, action };
							m_InputDelegateDelayed[i] << InputEvent{ i, action };
						}
					}
				}
			}
		}
	}

	void InputManager::HandleMouseHeldAndMovement(std::array<std::unordered_map<std::string, KeyboardMouseMappingContext>::iterator, 4> const& contexts)
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
						auto const& actions{ contexts[i]->second.mappedMouseActions[static_cast<size_t>(KeyInfo::ActionType::Held)] };
						auto const it{ actions.find(button) };
						if (it != end(actions))
						{
							for (auto const& action : it->second)
							{
								m_InputDelegateImmediate < InputEvent{ i, action };
								m_InputDelegateDelayed[i] << InputEvent{ i, action };
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

	void InputManager::HandleKeyboardHeld(std::array<std::unordered_map<std::string, KeyboardMouseMappingContext>::iterator, 4> const& contexts)
	{
		ME_PROFILE_FUNCTION()

		int numKeys{ };
		bool const* keyState{ SDL_GetKeyboardState(&numKeys) };
		if (numKeys > 0 && keyState)
		{
			for (uint32_t i{ 0 }; i < m_ActiveKeyboardMouseContexts.size(); ++i)
			{
				auto const& actions{ contexts[i]->second.mappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Held)]};
				for (auto const& keys : actions)
				{
					SDL_Scancode scancode{ SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(keys.first), NULL) };
					if (keyState[scancode])
					{
						for (auto const& action : keys.second)
						{
							m_InputDelegateImmediate < InputEvent{ i, action };
							m_InputDelegateDelayed[i] << InputEvent{ i, action };
						}
					}
				}
			}
		}
	}

	void InputManager::HandleGamepadHeld(std::array<std::unordered_map<std::string, GamepadMappingContext>::iterator, 4> const& contexts)
	{
		ME_PROFILE_FUNCTION()

		for (auto& g : m_Gamepads)
		{
			auto const& actions{ contexts[g.playerID]->second.mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::Held)]};
			for (auto const& buttons : actions)
			{
				SDL_GamepadButton const button{ static_cast<SDL_GamepadButton>(buttons.first) };
				if (SDL_GetGamepadButton(g.gamepad, button))
				{
					for (auto const& action : buttons.second)
					{
						m_InputDelegateImmediate < InputEvent{ g.playerID, action };
						m_InputDelegateDelayed[g.playerID] << InputEvent{ g.playerID, action };
					}
				}
			}
		}
	}

	void InputManager::HandleGamepadAxisState(std::array<std::unordered_map<std::string, GamepadMappingContext>::iterator, 4> const& contexts)
	{
		ME_PROFILE_FUNCTION()

		for (auto const& g : m_Gamepads)
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
						auto const& actions{ contexts[g.playerID]->second.mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::AxisStartHeld)] };
						auto const it{ actions.find(axis) };
						if (it != end(actions))
						{
							for (auto const& action : it->second)
							{
								m_InputDelegateImmediate < InputEvent{ g.playerID, action };
								m_InputDelegateDelayed[g.playerID] << InputEvent{ g.playerID, action };
							}
						}
					}

					m_GamepadAxes[g.playerID].held[axis] = true;

					auto const& actions{ contexts[g.playerID]->second.mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::AxisHeld)] };
					// held action
					auto const it{ actions.find(axis) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_InputDelegateImmediate < InputEvent{ g.playerID, action };
							m_InputDelegateDelayed[g.playerID] << InputEvent{ g.playerID, action };
						}
					}
				}
				else
				{
					if (m_GamepadAxes[g.playerID].held[axis])
					{
						// broadaast start held
						auto const& actions{ contexts[g.playerID]->second.mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::AxisReleased)] };
						auto const it{ actions.find(axis) };
						if (it != end(actions))
						{
							for (auto const& action : it->second)
							{
								m_InputDelegateImmediate < InputEvent{ g.playerID, action };
								m_InputDelegateDelayed[g.playerID] << InputEvent{ g.playerID, action };
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
				m_AvailablePlayerIDs_Gamepads.emplace_back(p.playerID);
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

		// Do all lookups here once so we dont do it per event in the loop
		std::array<std::unordered_map<std::string, KeyboardMouseMappingContext>::iterator, 4 > keyboardContexts{};
		std::array<std::unordered_map<std::string, GamepadMappingContext>::iterator, 4 > gamepadContexts{};
		for (uint32_t i{ 0 }; i < 4; ++i)
		{
			keyboardContexts[i] = m_KeyboardContexts.find(m_ActiveKeyboardMouseContexts[i]);
			gamepadContexts[i] = m_GamepadContexts.find(m_ActiveGamepadMappingContexts[i]);
		}

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			// Quit event -> return false so the gameloop can stop
			if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			{
				return false;
			}

			// Mouse
			HandleMouseAction(event, SDL_EVENT_MOUSE_BUTTON_DOWN, MouseInfo::ActionType::Down, keyboardContexts);
			HandleMouseAction(event, SDL_EVENT_MOUSE_BUTTON_UP, MouseInfo::ActionType::Up, keyboardContexts);
			HandleMouseAction(event, SDL_EVENT_MOUSE_WHEEL, MouseInfo::ActionType::Scrolled, keyboardContexts);
			HandleMouseAction(event, SDL_EVENT_MOUSE_MOTION, MouseInfo::ActionType::Moved, keyboardContexts);
			HandleMouseAction(event, SDL_EVENT_WINDOW_MOUSE_ENTER, MouseInfo::ActionType::EnteredWindow, keyboardContexts);
			HandleMouseAction(event, SDL_EVENT_WINDOW_MOUSE_LEAVE, MouseInfo::ActionType::LeftWindow, keyboardContexts);

			// Keyboard
			//Down this frame
			if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
			{
				for (uint32_t i{ 0 }; i < m_ActiveKeyboardMouseContexts.size(); ++i)
				{
					auto const& actions{ keyboardContexts[i]->second.mappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Down)]};
					auto const it{ actions.find(static_cast<uint32_t>(event.key.key)) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_InputDelegateImmediate < InputEvent{ i, action };
							m_InputDelegateDelayed[i] << InputEvent{ i, action };
						}
					}
				}
			}
			//Up this frame
			else if (event.type == SDL_EVENT_KEY_UP)
			{
				for (uint32_t i{ 0 }; i < m_ActiveKeyboardMouseContexts.size(); ++i)
				{
					auto const& actions{ keyboardContexts[i]->second.mappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Up)] };
					auto it{ actions.find(static_cast<uint32_t>(event.key.key)) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_InputDelegateImmediate < InputEvent{ i, action };
							m_InputDelegateDelayed[i] << InputEvent{ i, action };
						}
					}
				}
			}

			// Gamepad
			if (event.type == SDL_EVENT_GAMEPAD_ADDED)
			{
				if (not m_AvailablePlayerIDs_Gamepads.empty())
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
							m_Gamepads.emplace_back(gamepad, m_AvailablePlayerIDs_Gamepads.back());
							m_AvailablePlayerIDs_Gamepads.pop_back();

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
					auto const& actions{ gamepadContexts[playerID]->second.mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::Down)] };
					auto const it{ actions.find(static_cast<uint32_t>(event.gbutton.button)) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_InputDelegateImmediate < InputEvent{ static_cast<uint32_t>(playerID), action };
							m_InputDelegateDelayed[playerID] << InputEvent{ static_cast<uint32_t>(playerID), action };
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
					auto const& actions{ gamepadContexts[playerID]->second.mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::Up)] };
					auto const it{ actions.find(static_cast<uint32_t>(event.gbutton.button)) };
					if (it != end(actions))
					{
						for (auto const& action : it->second)
						{
							m_InputDelegateImmediate < InputEvent{ static_cast<uint32_t>(playerID), action };
							m_InputDelegateDelayed[playerID] << InputEvent{ static_cast<uint32_t>(playerID), action };
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
					auto const& actions{ gamepadContexts[playerID]->second.mappedGamepadActions[static_cast<size_t>(GamepadInfo::ActionType::AxisMoved)] };
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
									m_InputDelegateImmediate < InputEvent{ static_cast<uint32_t>(playerID), action };
									m_InputDelegateDelayed[playerID] << InputEvent{ static_cast<uint32_t>(playerID), action };
								}
							}
						}
					}
				}
			}
		}

		HandleMouseHeldAndMovement(keyboardContexts);
		HandleKeyboardHeld(keyboardContexts);
		HandleGamepadHeld(gamepadContexts);
		HandleGamepadAxisState(gamepadContexts);

		return true;
	}

	void InputManager::Destroy()
	{
		m_Players.clear();

		for (auto& p : m_Gamepads)
		{
			SDL_CloseGamepad(p.gamepad);
			m_AvailablePlayerIDs_Gamepads.emplace_back(p.playerID);
		}

		m_Gamepads.clear();
	}

	std::vector<Player const*> const InputManager::GetPlayers() const noexcept
	{
		// only max 4 elements, copies are fine
		std::vector<Player const*> temp;
		for (auto& p : m_Players)
		{
			temp.emplace_back(p.get());
		}
		return temp;
	}

	std::vector<Player*> InputManager::GetPlayers() noexcept
	{
		// only max 4 elements, copies are fine

		std::vector<Player*> temp;
		for (auto& p : m_Players)
		{
			temp.emplace_back(p.get());
		}
		return temp;
	}

	Player* InputManager::GetPlayer(uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3);
		// only max 4 elements, lookup like this is fine
		for (auto& p : m_Players)
		{
			if (p->PlayerID() == playerID)
			{
				return p.get();
			}
		}

		ME_LOG_ERROR(MauCor::LogCategory::Engine, "Trying to get a player for playerID {}, but player does not exist (nullptr return)", playerID);
		return nullptr;
	}

	bool InputManager::DestroyPlayer(uint32_t playerID) noexcept
	{
		if (1 == std::erase_if(m_Players, [playerID](auto& p) { return p->PlayerID() == playerID; }))
		{
			m_AvailablePlayerIDs.emplace_back(playerID);
			return true;
		}

		ME_LOG_ERROR(MauCor::LogCategory::Engine, "Trying to destroy player for playerID {} but player does not exist", playerID);
		return false;
	}

	bool InputManager::DestroyPlayer(Player const* player) noexcept
	{
		return DestroyPlayer(player->PlayerID());
	}

	uint32_t InputManager::NumPlayers() const noexcept
	{
		return static_cast<uint32_t>(m_Players.size());
	}

	void InputManager::SetMappingContext(std::string const& mappingContext, uint32_t playerID) noexcept
	{
		SetKeyboardMappingContext(mappingContext, playerID);
		SetGamepadMappingContext(mappingContext, playerID);
	}

	void InputManager::SetKeyboardMappingContext(std::string const& mappingContext, uint32_t playerID) noexcept
	{
		auto const it{ m_KeyboardContexts.find(mappingContext) };
		if (it == end(m_KeyboardContexts))
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Creating new mapping context in SetKeyboardMappingContext; {} was not created yet", mappingContext);
			auto insertedIt{ m_KeyboardContexts.emplace(mappingContext, KeyboardMouseMappingContext{}) };
			auto& context{ insertedIt.first->second };
			context.mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
			context.mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));
		}

		m_ActiveKeyboardMouseContexts[playerID] = mappingContext;
	}

	void InputManager::SetGamepadMappingContext(std::string const& mappingContext, uint32_t playerID) noexcept
	{
		auto const it{ m_GamepadContexts.find(mappingContext) };
		if (it == end(m_GamepadContexts))
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Creating new mapping context in SetGamepadMappingContext; {} was not created yet", mappingContext);
			auto insertedIt{ m_GamepadContexts.emplace(mappingContext, GamepadMappingContext{}) };
			auto& context{ insertedIt.first->second };
			context.mappedGamepadActions.resize(static_cast<size_t>(GamepadInfo::ActionType::COUNT));
		}

		m_ActiveGamepadMappingContexts[playerID] = mappingContext;	
	}

	std::string const& InputManager::GetKeyboardMappingContext(uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID < m_ActiveKeyboardMouseContexts.size());
		return m_ActiveKeyboardMouseContexts[playerID];
	}

	std::string const& InputManager::GetGamepadMappingContext(uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID < m_ActiveGamepadMappingContexts.size());
		return m_ActiveGamepadMappingContexts[playerID];
	}

	void InputManager::BindAction(std::string const& actionName, KeyInfo const& keyInfo, std::string const& mappingContext) noexcept
	{
		auto it{ m_KeyboardContexts.find(mappingContext) };
		if (it == end(m_KeyboardContexts))
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Creating new mapping context in BindAction; {} was not created yet", mappingContext);
			auto insertedIt{ m_KeyboardContexts.emplace(mappingContext, KeyboardMouseMappingContext{}) };
			auto& context{ insertedIt.first->second };
			context.mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
			context.mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));

			it = insertedIt.first;
		}

		auto& actionTypeMap{ it->second.mappedKeyboardActions[static_cast<size_t>(keyInfo.type)] };
		actionTypeMap[static_cast<uint32_t>(keyInfo.key)].emplace_back(actionName);
		it->second.actionToKeyboardKey[actionName].emplace_back(static_cast<uint32_t>(keyInfo.key));
	}

	void InputManager::BindAction(std::string const& actionName, MouseInfo const& mouseInfo, std::string const& mappingContext) noexcept
	{
		auto it{ m_KeyboardContexts.find(mappingContext) };
		if (it == end(m_KeyboardContexts))
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Creating new mapping context in BindAction; {} was not created yet", mappingContext);
			auto insertedIt{ m_KeyboardContexts.emplace(mappingContext, KeyboardMouseMappingContext{}) };
			auto& context{ insertedIt.first->second };
			context.mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
			context.mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));

			it = insertedIt.first;
		}

		auto& actionTypeMap{ it->second.mappedMouseActions[static_cast<size_t>(mouseInfo.type)] };
		actionTypeMap[mouseInfo.button].emplace_back(actionName);

		it->second.actionToMouseButton[actionName].emplace_back(mouseInfo.button);
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

		auto it{ m_GamepadContexts.find(mappingContext) };
		if (it == end(m_GamepadContexts))
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Creating new mapping context in BindAction; {} was not created yet", mappingContext);
			auto insertedIt{ m_GamepadContexts.emplace(mappingContext, GamepadMappingContext{}) };
			auto& context{ insertedIt.first->second };
			context.mappedGamepadActions.resize(static_cast<size_t>(GamepadInfo::ActionType::COUNT));

			it = insertedIt.first;
		}

		auto& actionTypeMap{ it->second.mappedGamepadActions[static_cast<size_t>(gamepadInfo.type)] };
		actionTypeMap[btnAxis].emplace_back(actionName);

		it->second.actionToGamepad[actionName].emplace_back(btnAxis);
	}

	void InputManager::UnBindActionGamepad(std::string const& actionName, std::string const& mappingContext) noexcept
	{
		auto const contextIt{ m_GamepadContexts.find(mappingContext) };
		if (contextIt != end(m_GamepadContexts))
		{
			auto const it{ contextIt->second.actionToGamepad.find(actionName) };
			if (it != end(contextIt->second.actionToGamepad))
			{
				for (auto&& b : it->second)
				{
					for (auto& mappedGamepad : contextIt->second.mappedGamepadActions)
					{
						// need to find the string and erase then
						std::erase_if(mappedGamepad[b],
							[&actionName](std::string const& a)
							{
								return a == actionName;
							});
					}
				}

				contextIt->second.actionToGamepad.erase(it);
			}
		}
		else
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to unbind action for mapping context that doesn't exist for gamepad: {}", mappingContext);
		}	
	}

	void InputManager::UnBindActionKeyboard(std::string const& actionName, std::string const& mappingContext) noexcept
	{
		auto const contextIt{ m_KeyboardContexts.find(mappingContext) };
		if (contextIt != end(m_KeyboardContexts))
		{
			// Mouse
			if (auto const it{ contextIt->second.actionToMouseButton.find(actionName) };
				it != end(contextIt->second.actionToMouseButton))
			{
				for (auto&& b : it->second)
				{
					for (auto& mappedMouse : contextIt->second.mappedMouseActions)
					{
						// need to find the string and erase then
						std::erase_if(mappedMouse[b],
							[&actionName](std::string const& a)
							{
								return a == actionName;
							});
					}
				}
				contextIt->second.actionToMouseButton.erase(it);
			}

			// Keyboard	
			if (auto const it{ contextIt->second.actionToKeyboardKey.find(actionName) };
				it != end(contextIt->second.actionToKeyboardKey))
			{
				for (auto&& b : it->second)
				{
					for (auto& mappedKeyboard : contextIt->second.mappedKeyboardActions)
					{
						std::erase_if(mappedKeyboard[b],
							[&actionName](std::string const& a)
							{
								return a == actionName;
							});
					}
				}
				contextIt->second.actionToKeyboardKey.erase(it);
			}
		}
		else
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to unbind action for mapping context that doesnt exist (keyboard & mouse) {}", mappingContext);
		}
	}

	void InputManager::UnBindAction(std::string const& actionName, std::string const& mappingContext) noexcept
	{
		UnBindActionGamepad(actionName, mappingContext);
		UnBindActionKeyboard(actionName, mappingContext);
	}

	void InputManager::UnBindAllActions(KeyInfo const& keyInfo, std::string const& mappingContext) noexcept
	{
		auto const contextIt{ m_KeyboardContexts.find(mappingContext) };
		if (contextIt == end(m_KeyboardContexts))
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to unbind all actions (for a given keyinfo) for a mapping context that does not exist: {}", mappingContext);
			return;
		}
		
		auto const type{ static_cast<uint32_t>(keyInfo.type) };
		auto const it{ contextIt->second.mappedKeyboardActions[type].find(keyInfo.key) };
		if (it == end(contextIt->second.mappedKeyboardActions[type]))
		{
			return;
		}

		// erase in actionToKeyboardKey
		for (auto&& a : it->second)
		{
			std::erase_if(contextIt->second.actionToKeyboardKey[a],
				[keyInfo](uint32_t const k)
				{
					return k == keyInfo.key;
				});
		}

		// erase in mapped
		contextIt->second.mappedKeyboardActions[type].erase(it);
	}

	void InputManager::UnBindAllActions(MouseInfo const& mouseInfo, std::string const& mappingContext) noexcept
	{
		auto const contextIt{ m_KeyboardContexts.find(mappingContext) };
		if (contextIt == end(m_KeyboardContexts))
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to unbind all actions (for a given mouseInfo) for a mapping context that does not exist: {}", mappingContext);
			return;
		}
	
		auto const type{ static_cast<uint32_t>(mouseInfo.type) };
		auto const it{ contextIt->second.mappedMouseActions[type].find(mouseInfo.button) };
		if (it == end(contextIt->second.mappedMouseActions[type]))
		{
			return;
		}

		// erase in actionToMouseButton
		for (auto&& a : it->second)
		{
			std::erase_if(contextIt->second.actionToMouseButton[a],
				[mouseInfo](uint8_t const b)
				{
					return b == mouseInfo.button;
				});
		}

		// erase in mapped
		contextIt->second.mappedMouseActions[type].erase(it);
	}

	void InputManager::UnBindAllActions(GamepadInfo const& gamepadInfo, std::string const& mappingContext) noexcept
	{
		auto const contextIt{ m_GamepadContexts.find(mappingContext) };
		if (contextIt == end(m_GamepadContexts))
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to unbind all actions (for a given gamepadinfo) for a mapping context that does not exist: {}", mappingContext);
			return;
		}
		
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

		auto const type{ static_cast<uint32_t>(gamepadInfo.type) };
		auto& mappedActions{ contextIt->second.mappedGamepadActions[type] };
		auto const it{ mappedActions.find(btnAxis) };
		if (it == end(mappedActions))
		{
			return;
		}

		// erase in actionToGamepad
		for (auto&& a : it->second)
		{
			std::erase_if(contextIt->second.actionToGamepad[a],
				[btnAxis](uint8_t const b)
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
		else
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to erase mapping context that does not exist: {}", mappingContext);
			return;
		}

		if (!m_KeyboardContexts.contains(newMappingContextIfErasedIsActive))
		{
			auto const insertedIt{ m_KeyboardContexts.emplace(newMappingContextIfErasedIsActive, KeyboardMouseMappingContext{}) };
			insertedIt.first->second.mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
			insertedIt.first->second.mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));
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
		else
		{
			ME_LOG_WARN(MauCor::LogCategory::Engine, "Trying to erase mapping context that does not exist: {}", mappingContext);
			return;
		}

		if (!m_GamepadContexts.contains(newMappingContextIfErasedIsActive))
		{
			auto const insertedIt{ m_GamepadContexts.emplace(newMappingContextIfErasedIsActive, GamepadMappingContext{}) };
			insertedIt.first->second.mappedGamepadActions.resize(static_cast<size_t>(GamepadInfo::ActionType::COUNT));
		}

		for (auto& c : m_ActiveGamepadMappingContexts)
		{
			if (c == mappingContext)
			{
				c = newMappingContextIfErasedIsActive;
			}
		}
	}


	bool InputManager::HasGamepadForPlayerID(uint32_t playerID) const noexcept
	{
		return std::ranges::any_of(
			m_Gamepads,
			[playerID](auto const& entry) { return entry.playerID == playerID; });
	}

	uint32_t InputManager::NumConnectedControllers() const noexcept
	{
		return static_cast<uint32_t>(m_Gamepads.size());
	}

	bool InputManager::IsActionExecuted(std::string const& actionName, uint32_t playerID) const noexcept
	{
		ME_ENGINE_ASSERT(playerID <= 3, "Engine only supports 4 players");
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

		{
			auto const insertedIt{ m_KeyboardContexts.emplace("DEFAULT", KeyboardMouseMappingContext{} ) };
			insertedIt.first->second.mappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
			insertedIt.first->second.mappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));
			m_ActiveKeyboardMouseContexts.resize(4);
			for (auto& a : m_ActiveKeyboardMouseContexts)
			{
				a = "DEFAULT";
			}
		}

		{
			auto const insertedIt{ m_GamepadContexts.emplace("DEFAULT", GamepadMappingContext{}) };
			insertedIt.first->second.mappedGamepadActions.resize(static_cast<size_t>(GamepadInfo::ActionType::COUNT));
			m_ActiveGamepadMappingContexts.resize(4);
			for (auto& a : m_ActiveGamepadMappingContexts)
			{
				a = "DEFAULT";
			}
		}

		ResetState();
	}
}
