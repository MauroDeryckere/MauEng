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
	class InputManager final : public MauCor::Singleton<InputManager>
	{
	public:
		void SetMappingContext(std::string const& mappingContext, uint32_t playerID = 0) noexcept;
		void SetKeyboardMappingContext(std::string const& mappingContext, uint32_t playerID = 0) noexcept;
		void SetGamepadMappingContext(std::string const& mappingContext, uint32_t playerID = 0) noexcept;

		[[nodiscard]] std::string const& GetKeyboardMappingContext(uint32_t playerID = 0) const noexcept;
		[[nodiscard]] std::string const& GetGamepadMappingContext(uint32_t playerID = 0) const noexcept;

		void BindAction(std::string const& actionName, KeyInfo const& keyInfo, std::string const& mappingContext = "DEFAULT") noexcept;
		void BindAction(std::string const& actionName, MouseInfo const& mouseInfo, std::string const& mappingContext = "DEFAULT") noexcept;
		void BindAction(std::string const& actionName, GamepadInfo const& gamepadInfo, std::string const& mappingContext = "DEFAULT") noexcept;
		void UnBindAction(std::string const& actionName, std::string const& mappingContext = "DEFAULT") noexcept;

		void UnBindAllActions(KeyInfo const& keyInfo, std::string const& mappingContext = "DEFAULT") noexcept;
		void UnBindAllActions(MouseInfo const& mouseInfo, std::string const& mappingContext = "DEFAULT") noexcept;
		void UnBindAllActions(GamepadInfo const& gamepadInfo, std::string const& mappingContext = "DEFAULT") noexcept;
		void UnBindAllActions(std::string const& mappingContext = "DEFAULT") noexcept;

		void EraseMappingContext(std::string const& mappingContext, std::string const& newMappingContextIfErasedIsActive) noexcept;
		void EraseKeyboardMappingContext(std::string const& mappingContext, std::string const& newMappingContextIfErasedIsActive) noexcept;
		void EraseGamepadMappingContext(std::string const& mappingContext, std::string const& newMappingContextIfErasedIsActive) noexcept;


		[[nodiscard]] bool HasControllerForPlayerID(uint32_t playerID) const noexcept;
		[[nodiscard]] uint32_t NumConnectedControllers() const noexcept;

		void Clear() noexcept;

		[[nodiscard]] bool IsActionExecuted(std::string const& actionName, uint32_t playerID = 0) const noexcept;

		[[nodiscard]] std::pair<float, float> GetMousePosition() const noexcept { return { m_MouseX, m_MouseY }; }
		[[nodiscard]] std::pair<float, float> GetDeltaMouseMovement() const noexcept { return { m_MouseDeltaX, m_MouseDeltaY }; }
		[[nodiscard]] std::pair<float, float> GetDeltaMouseScroll() const noexcept { return { m_MouseScrollX, m_MouseScrollY }; }

		[[nodiscard]] std::pair<float, float> GetLeftJoystick(uint32_t playerID = 0) const noexcept;
		[[nodiscard]] std::pair<float, float> GetDeltaLeftJoystick(uint32_t playerID = 0) const noexcept;

		[[nodiscard]] std::pair<float, float> GetRightJoystick(uint32_t playerID = 0) const noexcept;
		[[nodiscard]] std::pair<float, float> GetDeltaRightJoystick(uint32_t playerID = 0) const noexcept;

		[[nodiscard]] float GetLeftTrigger(uint32_t playerID = 0) const noexcept;
		[[nodiscard]] float GetDeltaLeftTrigger(uint32_t playerID = 0) const noexcept;

		[[nodiscard]] float GetRightTrigger(uint32_t playerID = 0) const noexcept;
		[[nodiscard]] float GetDeltaRightTrigger(uint32_t playerID = 0) const noexcept;

		void SetJoystickDeadzone(float newDeadzone) noexcept;
		void SetTriggerDeadzone(float newDeadzone) noexcept;

		InputManager(InputManager const&) = delete;
		InputManager(InputManager&&) = delete;
		InputManager& operator=(InputManager const&) = delete;
		InputManager& operator=(InputManager&&) = delete;

	private:
		friend class Singleton<InputManager>;
		InputManager();
		virtual ~InputManager() override = default;

		// make sure user cant call destroy or process input
		friend class Engine;
		// Internal function to process all input, returns if the application should close based on the processed input
		[[nodiscard]] bool ProcessInput() noexcept;
		void Destroy();

		// All executed actions this frame
		std::vector<std::unordered_set<std::string>> m_ExecutedActions{};

		struct KeyboardMouseMappingContext final
		{
			// ActionType[]
			// State of key <keyID, actions[ actionname ] >
			std::vector<std::unordered_map<uint32_t, std::vector<std::string>>> mappedKeyboardActions;
			std::vector<std::unordered_map<uint8_t, std::vector<std::string>>> mappedMouseActions;

			std::unordered_map<std::string, std::vector<uint32_t>> actionToKeyboardKey;
			std::unordered_map<std::string, std::vector<uint8_t>> actionToMouseButton;
		};
		struct GamepadMappingContext final
		{
			//ActionType[]
			// State of key <keyID, actions[ actionname ] >
			std::vector<std::unordered_map<uint32_t, std::vector<std::string>>> mappedGamepadActions;
			std::unordered_map<std::string, std::vector<uint32_t>> actionToGamepad;
		};

		std::vector<std::string> m_ActiveKeyboardMouseContexts{
			"DEFAULT", "DEFAULT", "DEFAULT", "DEFAULT"
		};

		std::vector<std::string> m_ActiveGamepadMappingContexts{
			"DEFAULT", "DEFAULT", "DEFAULT", "DEFAULT"
		};

		std::unordered_map<std::string, KeyboardMouseMappingContext> m_KeyboardContexts{};
		std::unordered_map<std::string, GamepadMappingContext> m_GamepadContexts{};

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
		
		struct GamepadAxisState final
		{
			// normalized [-1.0, 1.0]
			std::array<float, SDL_GAMEPAD_AXIS_COUNT> current{};
			std::array<float, SDL_GAMEPAD_AXIS_COUNT> delta{};
			std::array<bool, SDL_GAMEPAD_AXIS_COUNT> held{};
		};

		std::array<GamepadAxisState, 4> m_GamepadAxes{};

		float m_JoystickDeadzone{ .1f };
		float m_TriggerDeadzone{ .1f };


		void HandleMouseAction(SDL_Event const& event, Uint32 const evType, MouseInfo::ActionType const actType);
		void HandleMouseHeldAndMovement();
		void HandleKeyboardHeld();
		void HandleGamepadHeld();
		void HandleGamepadAxisState();

		void ResetState();
	};
}

#endif	