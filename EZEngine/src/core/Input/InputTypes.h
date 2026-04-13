#pragma once
#ifndef __CORE_INPUT_TYPES_H__
#define __CORE_INPUT_TYPES_H__

#include <cstdint>

namespace EZ
{
	enum class KeyCode : std::uint16_t
	{
		Unknown = 0,

		A, B, C, D, E, F, G, H, I, J, K, L, M,
		N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		Num0, Num1, Num2, Num3, Num4,
		Num5, Num6, Num7, Num8, Num9,

		F1, F2, F3, F4, F5, F6,
		F7, F8, F9, F10, F11, F12,

		Up,
		Down,
		Left,
		Right,

		Space,
		Enter,
		Escape,
		Tab,
		Backspace,

		LeftShift,
		RightShift,
		LeftCtrl,
		RightCtrl,
		LeftAlt,
		RightAlt,

		Insert,
		DeleteKey,
		Home,
		End,
		PageUp,
		PageDown,

		CapsLock,

		Count
	};

	enum class MouseButton : std::uint8_t
	{
		Left = 0,
		Right,
		Middle,
		X1,
		X2,
		Count
	};
}

#endif