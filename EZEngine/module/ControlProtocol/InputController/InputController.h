#pragma once
#ifndef __INPUT_CONTROLLER_H__
#define __INPUT_CONTROLLER_H__

#include <array>
#include <cstdint>

#include "core/Input/InputTypes.h"

namespace ControlProtocol
{
	struct ButtonState
	{
		bool down = false;
		bool pressed = false;
		bool released = false;
	};

	class InputController
	{
	public:
		static constexpr std::size_t KeyCount =
			static_cast<std::size_t>(EZ::KeyCode::Count);

		static constexpr std::size_t MouseButtonCount =
			static_cast<std::size_t>(EZ::MouseButton::Count);

	public:
		void BeginFrame();
		void EndFrame();

		void SetKeyDown(EZ::KeyCode key, bool isDown);
		bool IsKeyDown(EZ::KeyCode key) const;
		bool IsKeyPressed(EZ::KeyCode key) const;
		bool IsKeyReleased(EZ::KeyCode key) const;

		void SetMouseButtonDown(EZ::MouseButton button, bool isDown);
		bool IsMouseButtonDown(EZ::MouseButton button) const;
		bool IsMouseButtonPressed(EZ::MouseButton button) const;
		bool IsMouseButtonReleased(EZ::MouseButton button) const;

		void SetMousePosition(float x, float y);
		void SetMouseDelta(float dx, float dy);
		void AddMouseWheel(float x, float y);

		float GetMouseX() const { return m_MouseX; }
		float GetMouseY() const { return m_MouseY; }
		float GetMouseDeltaX() const { return m_MouseDeltaX; }
		float GetMouseDeltaY() const { return m_MouseDeltaY; }
		float GetWheelX() const { return m_WheelX; }
		float GetWheelY() const { return m_WheelY; }

		float GetMoveX() const { return m_MoveX; }
		float GetMoveY() const { return m_MoveY; }

		void RebuildAxes();

	private:
		static bool IsValidKey(EZ::KeyCode key);
		static bool IsValidMouseButton(EZ::MouseButton button);

	private:
		std::array<ButtonState, KeyCount> m_Keys{};
		std::array<ButtonState, MouseButtonCount> m_MouseButtons{};

		float m_MouseX = 0.0f;
		float m_MouseY = 0.0f;
		float m_MouseDeltaX = 0.0f;
		float m_MouseDeltaY = 0.0f;
		float m_WheelX = 0.0f;
		float m_WheelY = 0.0f;

		float m_MoveX = 0.0f;
		float m_MoveY = 0.0f;
	};
}

#endif