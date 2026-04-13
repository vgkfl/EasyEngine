#include "InputSystem.h"

#include <SDL3/SDL.h>

#include "ControlProtocol/InputController/InputController.h"
#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"
#include "core/Input/InputTypes.h"

namespace
{
	EZ::KeyCode MapSDLScancode(SDL_Scancode code)
	{
		switch (code)
		{
		case SDL_SCANCODE_A: return EZ::KeyCode::A;
		case SDL_SCANCODE_B: return EZ::KeyCode::B;
		case SDL_SCANCODE_C: return EZ::KeyCode::C;
		case SDL_SCANCODE_D: return EZ::KeyCode::D;
		case SDL_SCANCODE_E: return EZ::KeyCode::E;
		case SDL_SCANCODE_F: return EZ::KeyCode::F;
		case SDL_SCANCODE_G: return EZ::KeyCode::G;
		case SDL_SCANCODE_H: return EZ::KeyCode::H;
		case SDL_SCANCODE_I: return EZ::KeyCode::I;
		case SDL_SCANCODE_J: return EZ::KeyCode::J;
		case SDL_SCANCODE_K: return EZ::KeyCode::K;
		case SDL_SCANCODE_L: return EZ::KeyCode::L;
		case SDL_SCANCODE_M: return EZ::KeyCode::M;
		case SDL_SCANCODE_N: return EZ::KeyCode::N;
		case SDL_SCANCODE_O: return EZ::KeyCode::O;
		case SDL_SCANCODE_P: return EZ::KeyCode::P;
		case SDL_SCANCODE_Q: return EZ::KeyCode::Q;
		case SDL_SCANCODE_R: return EZ::KeyCode::R;
		case SDL_SCANCODE_S: return EZ::KeyCode::S;
		case SDL_SCANCODE_T: return EZ::KeyCode::T;
		case SDL_SCANCODE_U: return EZ::KeyCode::U;
		case SDL_SCANCODE_V: return EZ::KeyCode::V;
		case SDL_SCANCODE_W: return EZ::KeyCode::W;
		case SDL_SCANCODE_X: return EZ::KeyCode::X;
		case SDL_SCANCODE_Y: return EZ::KeyCode::Y;
		case SDL_SCANCODE_Z: return EZ::KeyCode::Z;

		case SDL_SCANCODE_0: return EZ::KeyCode::Num0;
		case SDL_SCANCODE_1: return EZ::KeyCode::Num1;
		case SDL_SCANCODE_2: return EZ::KeyCode::Num2;
		case SDL_SCANCODE_3: return EZ::KeyCode::Num3;
		case SDL_SCANCODE_4: return EZ::KeyCode::Num4;
		case SDL_SCANCODE_5: return EZ::KeyCode::Num5;
		case SDL_SCANCODE_6: return EZ::KeyCode::Num6;
		case SDL_SCANCODE_7: return EZ::KeyCode::Num7;
		case SDL_SCANCODE_8: return EZ::KeyCode::Num8;
		case SDL_SCANCODE_9: return EZ::KeyCode::Num9;

		case SDL_SCANCODE_F1: return EZ::KeyCode::F1;
		case SDL_SCANCODE_F2: return EZ::KeyCode::F2;
		case SDL_SCANCODE_F3: return EZ::KeyCode::F3;
		case SDL_SCANCODE_F4: return EZ::KeyCode::F4;
		case SDL_SCANCODE_F5: return EZ::KeyCode::F5;
		case SDL_SCANCODE_F6: return EZ::KeyCode::F6;
		case SDL_SCANCODE_F7: return EZ::KeyCode::F7;
		case SDL_SCANCODE_F8: return EZ::KeyCode::F8;
		case SDL_SCANCODE_F9: return EZ::KeyCode::F9;
		case SDL_SCANCODE_F10: return EZ::KeyCode::F10;
		case SDL_SCANCODE_F11: return EZ::KeyCode::F11;
		case SDL_SCANCODE_F12: return EZ::KeyCode::F12;

		case SDL_SCANCODE_UP: return EZ::KeyCode::Up;
		case SDL_SCANCODE_DOWN: return EZ::KeyCode::Down;
		case SDL_SCANCODE_LEFT: return EZ::KeyCode::Left;
		case SDL_SCANCODE_RIGHT: return EZ::KeyCode::Right;

		case SDL_SCANCODE_SPACE: return EZ::KeyCode::Space;
		case SDL_SCANCODE_RETURN: return EZ::KeyCode::Enter;
		case SDL_SCANCODE_ESCAPE: return EZ::KeyCode::Escape;
		case SDL_SCANCODE_TAB: return EZ::KeyCode::Tab;
		case SDL_SCANCODE_BACKSPACE: return EZ::KeyCode::Backspace;

		case SDL_SCANCODE_LSHIFT: return EZ::KeyCode::LeftShift;
		case SDL_SCANCODE_RSHIFT: return EZ::KeyCode::RightShift;
		case SDL_SCANCODE_LCTRL: return EZ::KeyCode::LeftCtrl;
		case SDL_SCANCODE_RCTRL: return EZ::KeyCode::RightCtrl;
		case SDL_SCANCODE_LALT: return EZ::KeyCode::LeftAlt;
		case SDL_SCANCODE_RALT: return EZ::KeyCode::RightAlt;

		case SDL_SCANCODE_INSERT: return EZ::KeyCode::Insert;
		case SDL_SCANCODE_DELETE: return EZ::KeyCode::DeleteKey;
		case SDL_SCANCODE_HOME: return EZ::KeyCode::Home;
		case SDL_SCANCODE_END: return EZ::KeyCode::End;
		case SDL_SCANCODE_PAGEUP: return EZ::KeyCode::PageUp;
		case SDL_SCANCODE_PAGEDOWN: return EZ::KeyCode::PageDown;

		case SDL_SCANCODE_CAPSLOCK: return EZ::KeyCode::CapsLock;

		default:
			return EZ::KeyCode::Unknown;
		}
	}

	EZ::MouseButton MapSDLMouseButton(std::uint8_t button)
	{
		switch (button)
		{
		case SDL_BUTTON_LEFT:   return EZ::MouseButton::Left;
		case SDL_BUTTON_RIGHT:  return EZ::MouseButton::Right;
		case SDL_BUTTON_MIDDLE: return EZ::MouseButton::Middle;
		case SDL_BUTTON_X1:     return EZ::MouseButton::X1;
		case SDL_BUTTON_X2:     return EZ::MouseButton::X2;
		default:                return EZ::MouseButton::Left;
		}
	}
}

void InputSystem::Update(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime)
{
	(void)project;
	(void)deltaTime;

	auto* input = world.TryGet<ControlProtocol::InputController>();
	if (!input)
	{
		return;
	}

	input->BeginFrame();

	SDL_PumpEvents();

	int keyCount = 0;
	const bool* keyboard = SDL_GetKeyboardState(&keyCount);
	if (keyboard)
	{
		for (int i = 0; i < keyCount; ++i)
		{
			const EZ::KeyCode key = MapSDLScancode(static_cast<SDL_Scancode>(i));
			if (key == EZ::KeyCode::Unknown)
			{
				continue;
			}

			input->SetKeyDown(key, keyboard[i]);
		}
	}

	float mouseX = 0.0f;
	float mouseY = 0.0f;
	SDL_GetMouseState(&mouseX, &mouseY);
	input->SetMousePosition(mouseX, mouseY);

	float mouseDX = 0.0f;
	float mouseDY = 0.0f;
	SDL_GetRelativeMouseState(&mouseDX, &mouseDY);
	input->SetMouseDelta(mouseDX, mouseDY);

	const SDL_MouseButtonFlags mouseButtons = SDL_GetMouseState(nullptr, nullptr);
	input->SetMouseButtonDown(EZ::MouseButton::Left, (mouseButtons & SDL_BUTTON_LMASK) != 0);
	input->SetMouseButtonDown(EZ::MouseButton::Middle, (mouseButtons & SDL_BUTTON_MMASK) != 0);
	input->SetMouseButtonDown(EZ::MouseButton::Right, (mouseButtons & SDL_BUTTON_RMASK) != 0);
	input->SetMouseButtonDown(EZ::MouseButton::X1, (mouseButtons & SDL_BUTTON_X1MASK) != 0);
	input->SetMouseButtonDown(EZ::MouseButton::X2, (mouseButtons & SDL_BUTTON_X2MASK) != 0);

	input->RebuildAxes();
}