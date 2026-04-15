#pragma once
#ifndef __TOOL_PERFORMANCE_PANEL_H__
#define __TOOL_PERFORMANCE_PANEL_H__

#include <array>

#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"

namespace Tool
{
	class PerformancePanel
	{
	public:
		void Draw(const EZ::ProjectContext& project, EZ::WorldContext& world);

	private:
		static constexpr int kHistoryCount = 180;

		std::array<float, kHistoryCount> m_FrameHistory{};
		int m_HistoryOffset = 0;
		bool m_Initialized = false;

		float m_WindowWidth = 800.0f;
		float m_WindowHeight = 500.0f;
		float m_NumberFontScale = 4.0f;
		float m_InfoFontScale = 2.0f;
		float m_PlotHeight = 220.0f;

		bool m_UseRealtimeDisplay = false;
		float m_DisplayRefreshInterval = 0.50f;

		float m_DisplayedFps = 0.0f;
		float m_DisplayedFrameMs = 0.0f;
		float m_DisplayRefreshAccumulator = 0.0f;
	};
}

#endif