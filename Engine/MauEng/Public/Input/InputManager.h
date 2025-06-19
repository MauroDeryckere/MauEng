#ifndef MAUENG_INPUTMANAGER_H
#define MAUENG_INPUTMANAGER_H

#include "Singleton.h"

#include "KeyInfo.h"
#include "MouseInfo.h"
#include "GamepadInfo.h"

#include <unordered_map>
#include <set>

#include "SDL3/SDL_events.h"

namespace MauEng
{
	// Keyboard only for now, no mapping contexts, just simple keybinding
	class InputManager final : public MauCor::Singleton<InputManager>
	{
	public:
		// Internal function to process all input, returns if the application should close based on the processed input
		[[nodiscard]] bool ProcessInput() noexcept;

		void Destroy();

		void BindAction(std::string const& actionName, KeyInfo const& keyInfo) noexcept;
		void BindAction(std::string const& actionName, MouseInfo const& mouseInfo) noexcept;
		void BindAction(std::string const& actionName, GamepadInfo const& gamepadInfo, uint32_t playerID = 0) noexcept;

		void UnBindAction(std::string const& actionName, uint32_t playerID = 0) noexcept;

		void UnBindAllActions(uint32_t playerID = 0) noexcept;

		//TODO does not consider the actiontype
		void UnBindAllActions(KeyInfo const& keyInfo) noexcept;
		void UnBindAllActions(MouseInfo const& mouseInfo) noexcept;
		// TODO gamepad variation of unbind all actions


		[[nodiscard]] bool HasControllerForPlayerID(uint32_t playerID) const noexcept;
		[[nodiscard]] uint32_t NumConnectedControllers() const noexcept;

		void Clear() noexcept;

		[[nodiscard]] bool IsActionExecuted(std::string const& actionName, uint32_t playerID = 0) const noexcept;

		[[nodiscard]] std::pair<float, float> GetMousePosition() const noexcept { return { m_MouseX, m_MouseY }; }
		[[nodiscard]] std::pair<float, float> GetDeltaMouseMovement() const noexcept { return { m_MouseDeltaX, m_MouseDeltaY }; }
		[[nodiscard]] std::pair<float, float> GetDeltaMouseScroll() const noexcept { return { m_MouseScrollX, m_MouseScrollY }; }

		InputManager(InputManager const&) = delete;
		InputManager(InputManager&&) = delete;
		InputManager& operator=(InputManager const&) = delete;
		InputManager& operator=(InputManager&&) = delete;

	private:
		friend class Singleton<InputManager>;
		InputManager();
		virtual ~InputManager() override = default;

		// All executed actions this frame
		std::vector<std::unordered_set<std::string>> m_ExecutedActions{};

		// ActionType[]
		// State of key <keyID, actions[ actionname ] >
		std::vector<std::unordered_map<uint32_t, std::vector<std::string>>> m_MappedKeyboardActions;
		std::vector<std::unordered_map<uint8_t, std::vector<std::string>>> m_MappedMouseActions;

		std::unordered_map<std::string, std::vector<uint32_t>> m_ActionToKeyboardKey;
		std::unordered_map<std::string, std::vector<uint8_t>> m_ActionToMouseButton;

		//PlayerID[]
		//ActionType[]
		// State of key <keyID, actions[ actionname ] >
		std::vector<std::vector<std::unordered_map<uint32_t, std::vector<std::string>>>> m_MappedGamepadActions;

		std::vector<std::unordered_map<std::string, std::vector<uint32_t>>> m_ActionToGamepad;

		struct Gamepad final
		{
			SDL_Gamepad* gamepad{ nullptr };
			uint32_t playerID{ UINT32_MAX };

			bool markedForRemove{ false };
		};

		std::vector<uint32_t> m_AvailablePlayerIDs { 3, 2, 1, 0 };
		std::vector<Gamepad> m_Gamepads{};

		float m_MouseX{ 0.f };
		float m_MouseY{ 0.f };
		float m_MouseDeltaX{ 0.f };
		float m_MouseDeltaY{ 0.f };

		float m_MouseScrollX{ 0.f };
		float m_MouseScrollY{ 0.f };


		void HandleMouseAction(SDL_Event const& event, Uint32 const evType, MouseInfo::ActionType const actType);
		void HandleMouseHeldAndMovement();
		void HandleKeyboardHeld();
		void HandleGamepadHeld();
		void ResetState();
	};
}

#endif	