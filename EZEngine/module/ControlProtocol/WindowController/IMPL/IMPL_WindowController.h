#pragma once
#ifndef __IMPL_WINDOWCONTROLLER_H__
#define __IMPL_WINDOWCONTROLLER_H__

#include <deque>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_properties.h>

#include "ControlProtocol/WindowController/WindowController.h"

namespace ControlProtocol
{
	struct WindowController::IMPL
	{
		SDL_Window* window = nullptr;
		SDL_WindowID windowID = 0;

		bool shouldClose = false;

		DataProtocol::WindowDesc desc{};
		std::deque<DataProtocol::WindowEventData> eventQueue;
	};
}

#endif