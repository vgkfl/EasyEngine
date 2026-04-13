#include "InputController.h"

namespace
{
	inline std::size_t ToIndex(EZ::KeyCode key)
	{
		return static_cast<std::size_t>(key);
	}

	inline std::size_t ToMouseIndex(EZ::MouseButton button)
	{
		return static_cast<std::size_t>(button);
	}
}

bool ControlProtocol::InputController::IsValidKey(EZ::KeyCode key)
{
	return key != EZ::KeyCode::Unknown &&
		static_cast<std::size_t>(key) < KeyCount;
}

bool ControlProtocol::InputController::IsValidMouseButton(EZ::MouseButton button)
{
	return static_cast<std::size_t>(button) < MouseButtonCount;
}

void ControlProtocol::InputController::BeginFrame()
{
	for (auto& key : m_Keys)
	{
		key.pressed = false;
		key.released = false;
	}

	for (auto& button : m_MouseButtons)
	{
		button.pressed = false;
		button.released = false;
	}

	m_MouseDeltaX = 0.0f;
	m_MouseDeltaY = 0.0f;
	m_WheelX = 0.0f;
	m_WheelY = 0.0f;

	m_MoveX = 0.0f;
	m_MoveY = 0.0f;
}

void ControlProtocol::InputController::EndFrame()
{
}

void ControlProtocol::InputController::SetKeyDown(EZ::KeyCode key, bool isDown)
{
	if (!IsValidKey(key))
	{
		return;
	}

	auto& state = m_Keys[ToIndex(key)];

	if (state.down != isDown)
	{
		if (isDown)
		{
			state.pressed = true;
		}
		else
		{
			state.released = true;
		}
	}

	state.down = isDown;
}

bool ControlProtocol::InputController::IsKeyDown(EZ::KeyCode key) const
{
	if (!IsValidKey(key))
	{
		return false;
	}

	return m_Keys[ToIndex(key)].down;
}

bool ControlProtocol::InputController::IsKeyPressed(EZ::KeyCode key) const
{
	if (!IsValidKey(key))
	{
		return false;
	}

	return m_Keys[ToIndex(key)].pressed;
}

bool ControlProtocol::InputController::IsKeyReleased(EZ::KeyCode key) const
{
	if (!IsValidKey(key))
	{
		return false;
	}

	return m_Keys[ToIndex(key)].released;
}

void ControlProtocol::InputController::SetMouseButtonDown(EZ::MouseButton button, bool isDown)
{
	if (!IsValidMouseButton(button))
	{
		return;
	}

	auto& state = m_MouseButtons[ToMouseIndex(button)];

	if (state.down != isDown)
	{
		if (isDown)
		{
			state.pressed = true;
		}
		else
		{
			state.released = true;
		}
	}

	state.down = isDown;
}

bool ControlProtocol::InputController::IsMouseButtonDown(EZ::MouseButton button) const
{
	if (!IsValidMouseButton(button))
	{
		return false;
	}

	return m_MouseButtons[ToMouseIndex(button)].down;
}

bool ControlProtocol::InputController::IsMouseButtonPressed(EZ::MouseButton button) const
{
	if (!IsValidMouseButton(button))
	{
		return false;
	}

	return m_MouseButtons[ToMouseIndex(button)].pressed;
}

bool ControlProtocol::InputController::IsMouseButtonReleased(EZ::MouseButton button) const
{
	if (!IsValidMouseButton(button))
	{
		return false;
	}

	return m_MouseButtons[ToMouseIndex(button)].released;
}

void ControlProtocol::InputController::SetMousePosition(float x, float y)
{
	m_MouseX = x;
	m_MouseY = y;
}

void ControlProtocol::InputController::SetMouseDelta(float dx, float dy)
{
	m_MouseDeltaX = dx;
	m_MouseDeltaY = dy;
}

void ControlProtocol::InputController::AddMouseWheel(float x, float y)
{
	m_WheelX += x;
	m_WheelY += y;
}

void ControlProtocol::InputController::RebuildAxes()
{
	m_MoveX = 0.0f;
	m_MoveY = 0.0f;

	if (IsKeyDown(EZ::KeyCode::A)) m_MoveX -= 1.0f;
	if (IsKeyDown(EZ::KeyCode::D)) m_MoveX += 1.0f;
	if (IsKeyDown(EZ::KeyCode::S)) m_MoveY -= 1.0f;
	if (IsKeyDown(EZ::KeyCode::W)) m_MoveY += 1.0f;
}