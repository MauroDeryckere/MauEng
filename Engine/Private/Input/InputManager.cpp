#include "EnginePCH.h"
#include "Input/InputManager.h"

#include <SDL3/SDL.h>

namespace MauEng
{
	InputManager::InputManager()
	{
		m_MappedActions.resize(static_cast<size_t>(KeyInfo::ActionType::COUNT));
	}

	bool InputManager::ProcessInput() noexcept
	{
		m_ExecutedActions.clear();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			{
				return false;
			}
			
			if (event.type == SDL_EVENT_KEY_DOWN)
			{
				auto const& actions{ m_MappedActions[static_cast<size_t>(KeyInfo::ActionType::Down)] };
				auto it{ actions.find(static_cast<uint32_t>(event.key.key)) };
				if (it != end(actions))
				{
					for (auto const& action : it->second)
					{
						m_ExecutedActions.emplace(action);
					}
				}
			}

			if (event.type == SDL_EVENT_KEY_UP)
			{
				auto const& actions{ m_MappedActions[static_cast<size_t>(KeyInfo::ActionType::Up)]};
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

		auto const& actions{ m_MappedActions[static_cast<size_t>(KeyInfo::ActionType::Held)] };

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

		return true;
	}

	bool InputManager::BindAction(std::string const& actionName, KeyInfo const& keyInfo) noexcept
	{
		auto& actionTypeVec{ m_MappedActions[static_cast<size_t>(keyInfo.type)] };
		auto it{ actionTypeVec.find(static_cast<uint32_t>(keyInfo.key)) };

		if (it != end(actionTypeVec))
		{
			// Action already bound
			return false;
		}

		actionTypeVec[static_cast<uint32_t>(keyInfo.key)].emplace_back(actionName);
		return true;
	}	

	bool InputManager::IsActionExecuted(std::string const& actionName) const noexcept
	{
		return m_ExecutedActions.contains(actionName);
	}
}
