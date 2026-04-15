#include "Tool/Editor/Panels/PerformancePanel.h"

#include <algorithm>

#include "imgui/imgui.h"

#include "System/RenderSystem/RenderSystem.h"
#include "Tool/Editor/EditorContext.h"

namespace Tool
{
	void PerformancePanel::Draw(const EZ::ProjectContext& project, EZ::WorldContext& world)
	{
		auto* editor = world.TryGet<Tool::EditorContext>();
		auto* renderSystem = world.TryGet<RenderSystem>();

		if (!editor || !editor->enabled || !editor->showPerformancePanel)
		{
			return;
		}

		EZ::u32 drawCalls = 0;
		EZ::u32 staticDrawCalls = 0;
		EZ::u32 skinnedDrawCalls = 0;
		EZ::u32 staticInstances = 0;
		EZ::u32 skinnedInstances = 0;

		if (renderSystem)
		{
			const auto& stats = renderSystem->GetFrameStats();
			drawCalls = stats.drawCalls;
			staticDrawCalls = stats.staticDrawCalls;
			skinnedDrawCalls = stats.skinnedDrawCalls;
			staticInstances = stats.staticInstanceCount;
			skinnedInstances = stats.skinnedInstanceCount;
		}

		const float logicDeltaTime = (project.deltaTime > 0.000001f)
			? project.deltaTime
			: (1.0f / 60.0f);

		const float logicFrameMs = logicDeltaTime * 1000.0f;
		const float logicFps = (logicDeltaTime > 0.000001f)
			? (1.0f / logicDeltaTime)
			: 0.0f;

		if (!m_Initialized)
		{
			m_FrameHistory.fill(logicFrameMs);
			m_DisplayedFps = logicFps;
			m_DisplayedFrameMs = logicFrameMs;
			m_Initialized = true;
		}

		m_FrameHistory[m_HistoryOffset] = logicFrameMs;
		m_HistoryOffset = (m_HistoryOffset + 1) % kHistoryCount;

		if (m_UseRealtimeDisplay)
		{
			m_DisplayedFps = logicFps;
			m_DisplayedFrameMs = logicFrameMs;
			m_DisplayRefreshAccumulator = 0.0f;
		}
		else
		{
			m_DisplayRefreshAccumulator += logicDeltaTime;
			if (m_DisplayRefreshAccumulator >= m_DisplayRefreshInterval)
			{
				m_DisplayedFps = logicFps;
				m_DisplayedFrameMs = logicFrameMs;
				m_DisplayRefreshAccumulator = 0.0f;
			}
		}

		float minMs = m_FrameHistory[0];
		float maxMs = m_FrameHistory[0];
		float avgMs = 0.0f;

		for (float value : m_FrameHistory)
		{
			minMs = std::min(minMs, value);
			maxMs = std::max(maxMs, value);
			avgMs += value;
		}

		avgMs /= static_cast<float>(kHistoryCount);

		float rangeMin = minMs;
		float rangeMax = maxMs;

		if ((rangeMax - rangeMin) < 0.2f)
		{
			const float center = 0.5f * (rangeMin + rangeMax);
			rangeMin = center - 0.1f;
			rangeMax = center + 0.1f;
		}

		const float padding = std::max(0.15f, (rangeMax - rangeMin) * 0.25f);
		rangeMin = std::max(0.0f, rangeMin - padding);
		rangeMax += padding;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(
			ImVec2(
				viewport->WorkPos.x + viewport->WorkSize.x - m_WindowWidth - 20.0f,
				viewport->WorkPos.y + 20.0f),
			ImGuiCond_FirstUseEver);

		ImGui::SetNextWindowSize(
			ImVec2(m_WindowWidth, m_WindowHeight),
			ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Performance Monitor", &editor->showPerformancePanel))
		{
			ImGui::SetWindowFontScale(m_NumberFontScale);
			ImGui::Text("FPS %.0f", m_DisplayedFps);
			ImGui::Text("%.2f ms", m_DisplayedFrameMs);
			ImGui::SetWindowFontScale(1.0f);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::SetWindowFontScale(m_InfoFontScale);
			ImGui::Text("Draw Calls %u", drawCalls);
			ImGui::Text("Static %u  (instances %u)", staticDrawCalls, staticInstances);
			ImGui::Text("Skinned %u (instances %u)", skinnedDrawCalls, skinnedInstances);
			ImGui::SetWindowFontScale(1.0f);

			ImGui::Spacing();

			ImGui::SetWindowFontScale(m_InfoFontScale);
			ImGui::Text("avg %.2f ms", avgMs);
			ImGui::SameLine(180.0f);
			ImGui::Text("min %.2f", minMs);
			ImGui::SameLine(320.0f);
			ImGui::Text("max %.2f", maxMs);
			ImGui::SetWindowFontScale(1.0f);

			ImGui::Spacing();

			ImGui::PushItemWidth(-1.0f);
			ImGui::PlotLines(
				"##LogicFrameTimeHistory",
				m_FrameHistory.data(),
				kHistoryCount,
				m_HistoryOffset,
				nullptr,
				rangeMin,
				rangeMax,
				ImVec2(0.0f, m_PlotHeight));
			ImGui::PopItemWidth();

			ImGui::Text("Logic Frame Time History");

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("Display Settings");
			ImGui::SliderFloat("Window Width", &m_WindowWidth, 320.0f, 1000.0f, "%.0f");
			ImGui::SliderFloat("Window Height", &m_WindowHeight, 220.0f, 700.0f, "%.0f");
			ImGui::SliderFloat("Number Font Scale", &m_NumberFontScale, 1.5f, 4.0f, "%.2f");
			ImGui::SliderFloat("Info Font Scale", &m_InfoFontScale, 0.8f, 2.5f, "%.2f");
			ImGui::SliderFloat("Plot Height", &m_PlotHeight, 60.0f, 320.0f, "%.0f");

			ImGui::Checkbox("Realtime FPS/MS", &m_UseRealtimeDisplay);
			if (!m_UseRealtimeDisplay)
			{
				ImGui::SliderFloat("Refresh Interval (s)", &m_DisplayRefreshInterval, 0.10f, 3.00f, "%.2f");
			}
		}
		ImGui::End();
	}
}