#include "EnginePCH.h"
#include "Input/InputManager.h"

#include <SDL3/SDL.h>

namespace MauEng
{
	InputManager::InputManager()
	{
		m_MappedKeyboardActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
		m_MappedMouseActions.resize(static_cast<size_t>(MouseInfo::ActionType::COUNT));
	}

	void InputManager::HandleMouseHeldAndMovement()
	{
		ME_PROFILE_FUNCTION();

		float x{ m_MouseX };
		float y{ m_MouseY };
		SDL_MouseButtonFlags mouseButtonState{ SDL_GetMouseState(&x, &y) };

		auto const& actions{ m_MappedMouseActions[static_cast<size_t>(KeyInfo::ActionType::Held)] };
		auto handleMouseBtnHeld = [&](int const mask, uint8_t const button)
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
			};

		handleMouseBtnHeld(SDL_BUTTON_LMASK, SDL_BUTTON_LEFT);
		handleMouseBtnHeld(SDL_BUTTON_RMASK, SDL_BUTTON_RIGHT);
		handleMouseBtnHeld(SDL_BUTTON_X1MASK, SDL_BUTTON_X1);
		handleMouseBtnHeld(SDL_BUTTON_X2MASK, SDL_BUTTON_X2);
		handleMouseBtnHeld(SDL_BUTTON_MMASK, SDL_BUTTON_MIDDLE);
	}

	void InputManager::HandleKeyboardHeld()
	{
		ME_PROFILE_FUNCTION();

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

	bool InputManager::ProcessInput() noexcept
	{
		ME_PROFILE_FUNCTION();

		// Reset all state that should be reset
		m_ExecutedActions.clear();

		m_MouseDeltaX = 0.f;
		m_MouseDeltaY = 0.f;
		m_MouseScrollX = 0.f;
		m_MouseScrollY = 0.f;


		auto const mouseActionfunc = [&](SDL_Event const& event, Uint32 const evType, MouseInfo::ActionType const actType)
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
					float scrollX{ event.wheel.x };
					float scrollY{ event.wheel.y };

					m_MouseScrollX = scrollX;
					m_MouseScrollY = scrollY;
				}

				auto const& actions{ m_MappedMouseActions[static_cast<size_t>(actType)] };
				
				auto it{ actions.find(
						evType == SDL_EVENT_MOUSE_WHEEL || evType == SDL_EVENT_MOUSE_MOTION || evType == SDL_EVENT_WINDOW_MOUSE_ENTER || evType == SDL_EVENT_WINDOW_MOUSE_LEAVE 
						? 0
						: event.button.button)};

				if (it != end(actions))
				{
					for (auto const& action : it->second)
					{
						m_ExecutedActions.emplace(action);
					}
				}
			}
		};

		{
			ME_PROFILE_SCOPE("Process events")
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
				{
					return false;
				}

				// Mouse
				mouseActionfunc(event, SDL_EVENT_MOUSE_BUTTON_DOWN, MouseInfo::ActionType::Down);
				mouseActionfunc(event, SDL_EVENT_MOUSE_BUTTON_UP, MouseInfo::ActionType::Up);
				mouseActionfunc(event, SDL_EVENT_MOUSE_WHEEL, MouseInfo::ActionType::Scrolled);
				mouseActionfunc(event, SDL_EVENT_MOUSE_MOTION, MouseInfo::ActionType::Moved);
				mouseActionfunc(event, SDL_EVENT_WINDOW_MOUSE_ENTER, MouseInfo::ActionType::EnteredWindow);
				mouseActionfunc(event, SDL_EVENT_WINDOW_MOUSE_LEAVE, MouseInfo::ActionType::LeftWindow);

				// Keyboard
				if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
				{
					auto const& actions{ m_MappedKeyboardActions[static_cast<size_t>(KeyInfo::ActionType::Down)] };
					auto it{ actions.find(static_cast<uint32_t>(event.key.key)) };
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
			}
		}

	

		HandleMouseHeldAndMovement();
		HandleKeyboardHeld();
		return true;
	}

	bool InputManager::BindAction(std::string const& actionName, KeyInfo const& keyInfo) noexcept
	{
		auto& actionTypeVec{ m_MappedKeyboardActions[static_cast<size_t>(keyInfo.type)] };
		actionTypeVec[static_cast<uint32_t>(keyInfo.key)].emplace_back(actionName);
		return true;
	}

	bool InputManager::BindAction(std::string const& actionName, MouseInfo const& mouseInfo) noexcept
	{
		auto& actionTypeVec{ m_MappedMouseActions[static_cast<size_t>(mouseInfo.type)] };
		actionTypeVec[mouseInfo.button].emplace_back(actionName);
		return true;
	}

	bool InputManager::IsActionExecuted(std::string const& actionName) const noexcept
	{
		return m_ExecutedActions.contains(actionName);
	}
}
