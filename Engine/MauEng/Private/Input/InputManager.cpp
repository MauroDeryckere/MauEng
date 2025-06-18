#include "EnginePCH.h"
#include "Input/InputManager.h"

#include <SDL3/SDL.h>

namespace MauEng
{
	InputManager::InputManager()
	{
		m_MappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
		m_MappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));

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
					m_ExecutedActions.emplace(action);
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
							m_ExecutedActions.emplace(action);
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
						m_ExecutedActions.emplace(action);
					}
				}
			}
		}
	}

	void InputManager::ResetState()
	{
		m_ExecutedActions.clear();

		m_MouseDeltaX = 0.f;
		m_MouseDeltaY = 0.f;
		m_MouseScrollX = 0.f;
		m_MouseScrollY = 0.f;

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
			if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
			{
				auto const& actions{ m_MappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Down)] };
				auto const it{ actions.find(static_cast<uint32_t>(event.key.key)) };
				if (it != end(actions))
				{
					for (auto const& action : it->second)
					{
						m_ExecutedActions.emplace(action);
					}
				}
			}
			else if (event.type == SDL_EVENT_KEY_UP)
			{
				auto const& actions{ m_MappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Up)] };
				auto it{ actions.find(static_cast<uint32_t>(event.key.key)) };
				if (it != end(actions))
				{
					for (auto const& action : it->second)
					{
						m_ExecutedActions.emplace(action);
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
		}

		HandleMouseHeldAndMovement();
		HandleKeyboardHeld();

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

	void InputManager::UnBindAction(std::string const& actionName) noexcept
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

	bool InputManager::IsActionExecuted(std::string const& actionName) const noexcept
	{
		return m_ExecutedActions.contains(actionName);
	}

	void InputManager::Clear() noexcept
	{
		m_MappedKeyboardActions.clear();
		m_ActionToKeyboardKey.clear();

		m_MappedMouseActions.clear();
		m_ActionToMouseButton.clear();

		m_ExecutedActions.clear();

		ResetState();
	}

}
