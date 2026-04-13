#pragma once
#ifndef __TOOL_IMGUI_LAYER_H__
#define __TOOL_IMGUI_LAYER_H__

#include "ControlProtocol/WindowController/WindowController.h"

namespace EZ
{
	struct WorldContext;
}

namespace Tool
{
	class ImGuiLayer
	{
	public:
		bool Initialize(EZ::WorldContext& world);
		void Shutdown();

		void BeginFrame();
		void Render();

		bool IsInitialized() const
		{
			return m_Initialized;
		}

	private:
		static void RawEventBridge(const void* backendEvent, void* userData);

	private:
		bool m_Initialized = false;
		bool m_FrameBegun = false;

		ControlProtocol::WindowController* m_Window = nullptr;
		ControlProtocol::WindowController::RawEventListenerHandle m_RawEventHandle{};
	};
}

#endif