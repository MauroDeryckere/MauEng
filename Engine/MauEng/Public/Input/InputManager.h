#ifndef MAUENG_INPUTMANAGER_H
#define MAUENG_INPUTMANAGER_H

#include "Singleton.h"

#include "KeyInfo.h"
#include "MouseInfo.h"

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
		bool BindAction(std::string const& actionName, MouseInfo const& mouseInfo) noexcept;
		// TODO unbinding

		[[nodiscard]] bool IsActionExecuted(std::string const& actionName) const noexcept;

		[[nodiscard]] std::pair<float, float> GetMousePosition() const noexcept
		{
			return { m_MouseX, m_MouseY };
		}

		[[nodiscard]] std::pair<float, float> GetDeltaMouseMovement() const noexcept
		{
			return { m_MouseDeltaX, m_MouseDeltaY };
		}

		[[nodiscard]] std::pair<float, float> GetDeltaMouseScroll() const noexcept
		{
			return { m_MouseScrollX, m_MouseScrollY };
		}

		InputManager(InputManager const&) = delete;
		InputManager(InputManager&&) = delete;
		InputManager& operator=(InputManager const&) = delete;
		InputManager& operator=(InputManager&&) = delete;

	private:
		friend class Singleton<InputManager>;
		InputManager();
		virtual ~InputManager() override = default;

		// Tolerance for the mouse moved state
		//float const MovementTolerance{ 1.f };

		// All executed actions this frame
		std::set<std::string> m_ExecutedActions;

		// State of key <keyID, actions[ actionname ] >
		std::vector<std::unordered_map<uint32_t, std::vector<std::string>>> m_MappedKeyboardActions;
		std::vector<std::unordered_map<uint8_t, std::vector<std::string>>> m_MappedMouseActions;

		float m_MouseX{ 0.f };
		float m_MouseY{ 0.f };
		float m_MouseDeltaX{ 0.f };
		float m_MouseDeltaY{ 0.f };

		float m_MouseScrollX{ 0.f };
		float m_MouseScrollY{ 0.f };

		void HandleMouseHeldAndMovement();
		void HandleKeyboardHeld();
	};
}

#endif	