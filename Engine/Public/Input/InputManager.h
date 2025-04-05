#ifndef MAUENG_INPUTMANAGER_H
#define MAUENG_INPUTMANAGER_H

#include "Singleton.h"

#include "KeyInfo.h"

#include <unordered_map>
#include <set>

namespace MauEng
{
	// Keyboard only for now, no mapping contexts, just simple keybinding
	class InputManager final : public MauCor::Singleton<InputManager>
	{
	public:
		// Internal function to process all input, returns if the application should close based on the processed input
		[[nodiscard]] bool ProcessInput() noexcept;

		bool BindAction(std::string const& actionName, KeyInfo const& keyInfo) noexcept;
		// TODO unbinding

		[[nodiscard]] bool IsActionExecuted(std::string const& actionName) const noexcept;


		InputManager(InputManager const&) = delete;
		InputManager(InputManager&&) = delete;
		InputManager& operator=(InputManager const&) = delete;
		InputManager& operator=(InputManager&&) = delete;

	private:
		friend class Singleton<InputManager>;
		InputManager();
		virtual ~InputManager() override = default;

		// All executed actions this frame
		std::set<std::string> m_ExecutedActions;

		// State of key <keyID, actions[ actionname ] >
		std::vector<std::unordered_map<uint32_t, std::vector<std::string>>> m_MappedActions;
	};
}

#endif	