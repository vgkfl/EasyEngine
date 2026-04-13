#include "EditorOverlayRenderFeature.h"

#include <string>
#include <vector>

#include "imgui/imgui.h"

#include "ControlProtocol/RenderPipelineController/RenderImageBuffer.h"
#include "ControlProtocol/RenderDeviceController/RenderDeviceController.h"
#include "Launcher/GameLauncher/Scene/SceneManager.h"
#include "Tool/Editor/EditorContext.h"
#include "Tool/Editor/ImGui/ImGuiLayer.h"
#include "Tool/Editor/Panels/InspectorPanel.h"
#include "Tool/Editor/Panels/WorldHierarchyPanel.h"

#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"
#include "core/Render/RenderTypes.h"
#include <System/RenderSystem/RenderSystem.h>

namespace
{
	bool EnsurePersistentFrameTarget(
		EZ::RenderImageTag tag,
		const EZ::RenderImageDesc& desc,
		ControlProtocol::RenderImageBuffer& imageBuffer,
		ControlProtocol::RenderDeviceController& device)
	{
		auto& slot = imageBuffer.GetOrCreate(tag);

		bool needRecreate = false;
		needRecreate =
			slot.desc.extent.width != desc.extent.width ||
			slot.desc.extent.height != desc.extent.height ||
			slot.desc.format != desc.format ||
			slot.desc.imported != desc.imported;

		if (needRecreate)
		{
			device.DestroyRenderTarget(slot);
			device.DestroyImage(slot);
		}

		slot.desc = desc;
		slot.desc.tag = tag;
		slot.desc.valid = true;
		slot.desc.persistent = true;

		return device.EnsureImage(slot);
	}


	void DrawFpsOverlay(const EZ::ProjectContext& project, EZ::WorldContext& world)
	{
		auto* renderSystem = world.TryGet<RenderSystem>();

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

		static constexpr int kHistoryCount = 180;
		static std::array<float, kHistoryCount> frameHistory{};
		static int historyOffset = 0;
		static bool initialized = false;

		// 可调参数
		static float s_WindowWidth = 800.0f;
		static float s_WindowHeight = 500.0f;
		static float s_NumberFontScale = 4.0f;
		static float s_InfoFontScale = 2.0f;
		static float s_PlotHeight = 220.0f;

		static bool s_UseRealtimeDisplay = false;
		static float s_DisplayRefreshInterval = 0.50f;

		// 用于“显示层”的缓存值
		static float s_DisplayedFps = 0.0f;
		static float s_DisplayedFrameMs = 0.0f;
		static float s_DisplayRefreshAccumulator = 0.0f;

		if (!initialized)
		{
			frameHistory.fill(logicFrameMs);
			s_DisplayedFps = logicFps;
			s_DisplayedFrameMs = logicFrameMs;
			initialized = true;
		}

		frameHistory[historyOffset] = logicFrameMs;
		historyOffset = (historyOffset + 1) % kHistoryCount;

		// 控制“显示值”是实时刷新还是间隔刷新
		if (s_UseRealtimeDisplay)
		{
			s_DisplayedFps = logicFps;
			s_DisplayedFrameMs = logicFrameMs;
			s_DisplayRefreshAccumulator = 0.0f;
		}
		else
		{
			s_DisplayRefreshAccumulator += logicDeltaTime;
			if (s_DisplayRefreshAccumulator >= s_DisplayRefreshInterval)
			{
				s_DisplayedFps = logicFps;
				s_DisplayedFrameMs = logicFrameMs;
				s_DisplayRefreshAccumulator = 0.0f;
			}
		}

		float minMs = frameHistory[0];
		float maxMs = frameHistory[0];
		float avgMs = 0.0f;

		for (float v : frameHistory)
		{
			minMs = std::min(minMs, v);
			maxMs = std::max(maxMs, v);
			avgMs += v;
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
		rangeMax = rangeMax + padding;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const ImVec2 windowPos(
			viewport->WorkPos.x + viewport->WorkSize.x - s_WindowWidth - 20.0f,
			viewport->WorkPos.y + 20.0f);

		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(s_WindowWidth, s_WindowHeight), ImGuiCond_Always);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::Begin("Performance Monitor", nullptr, flags))
		{
			ImGui::SetWindowFontScale(s_NumberFontScale);
			ImGui::Text("FPS %.0f", s_DisplayedFps);
			ImGui::Text("%.2f ms", s_DisplayedFrameMs);
			ImGui::SetWindowFontScale(1.0f);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::SetWindowFontScale(s_InfoFontScale);
			ImGui::Text("Draw Calls %u", drawCalls);
			ImGui::Text("Static %u  (instances %u)", staticDrawCalls, staticInstances);
			ImGui::Text("Skinned %u (instances %u)", skinnedDrawCalls, skinnedInstances);
			ImGui::SetWindowFontScale(1.0f);

			ImGui::SetWindowFontScale(s_InfoFontScale);
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
				frameHistory.data(),
				kHistoryCount,
				historyOffset,
				nullptr,
				rangeMin,
				rangeMax,
				ImVec2(0.0f, s_PlotHeight));
			ImGui::PopItemWidth();

			ImGui::Spacing();
			ImGui::Text("Logic Frame Time History");

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("Display Settings");
			ImGui::SliderFloat("Window Width", &s_WindowWidth, 320.0f, 800.0f, "%.0f");
			ImGui::SliderFloat("Window Height", &s_WindowHeight, 220.0f, 500.0f, "%.0f");
			ImGui::SliderFloat("Number Font Scale", &s_NumberFontScale, 1.5f, 4.0f, "%.2f");
			ImGui::SliderFloat("Info Font Scale", &s_InfoFontScale, 0.8f, 2.0f, "%.2f");
			ImGui::SliderFloat("Plot Height", &s_PlotHeight, 60.0f, 220.0f, "%.0f");

			ImGui::Spacing();
			ImGui::Checkbox("Realtime FPS/MS", &s_UseRealtimeDisplay);

			if (!s_UseRealtimeDisplay)
			{
				ImGui::SliderFloat("Refresh Interval (s)", &s_DisplayRefreshInterval, 0.10f, 3.00f, "%.2f");
			}
		}
		ImGui::End();
	}

}

namespace Tool
{
	void EditorOverlayRenderFeature::Setup(
		EZ::ProjectContext& project,
		EZ::WorldContext& world,
		ControlProtocol::RenderImageBuffer& imageBuffer,
		ControlProtocol::RenderDeviceController& device)
	{
		(void)project;

		auto* editor = world.TryGet<Tool::EditorContext>();
		if (!editor || !editor->enabled)
		{
			return;
		}

		auto* src = imageBuffer.TryGet(EZ::RenderImageTag::ForwardOpaque);
		if (!src)
		{
			return;
		}

		EZ::RenderImageDesc finalDesc = src->desc;
		finalDesc.source = EZ::RenderSourceType::UIPass;
		finalDesc.tag = EZ::RenderImageTag::FinalColor;
		finalDesc.valid = true;
		finalDesc.imported = false;
		finalDesc.persistent = true;
		finalDesc.sampled = true;
		finalDesc.colorAttachment = true;
		finalDesc.depthStencilAttachment = false;
		finalDesc.loadAction = EZ::RenderLoadAction::Load;
		finalDesc.storeAction = EZ::RenderStoreAction::Store;

		EnsurePersistentFrameTarget(
			EZ::RenderImageTag::FinalColor,
			finalDesc,
			imageBuffer,
			device);
	}

	void EditorOverlayRenderFeature::Execute(
		EZ::ProjectContext& project,
		EZ::WorldContext& world,
		ControlProtocol::RenderImageBuffer& imageBuffer,
		ControlProtocol::RenderDeviceController& device)
	{
		(void)project;

		auto* editor = world.TryGet<Tool::EditorContext>();
		auto* imguiLayer = world.TryGet<Tool::ImGuiLayer>();
		auto* hierarchyPanel = world.TryGet<Tool::WorldHierarchyPanel>();
		auto* inspectorPanel = world.TryGet<Tool::InspectorPanel>();
		auto* sceneManager = world.TryGet<GameScene::SceneManager>();

		if (!editor || !editor->enabled || !imguiLayer || !imguiLayer->IsInitialized())
		{
			return;
		}

		auto* src = imageBuffer.TryGet(EZ::RenderImageTag::ForwardOpaque);
		auto* dst = imageBuffer.TryGet(EZ::RenderImageTag::FinalColor);
		if (!src || !dst)
		{
			return;
		}

		if (!device.Blit(*src, *dst))
		{
			return;
		}

		if (!device.BeginPass(dst, nullptr))
		{
			return;
		}

		imguiLayer->BeginFrame();

		std::string requestedSceneId;
		bool requestReloadScene = false;

		if (ImGui::Begin("Editor"))
		{
			ImGui::Checkbox("World Hierarchy", &editor->showWorldHierarchy);
			ImGui::Checkbox("Inspector", &editor->showInspector);
			ImGui::Checkbox("Demo Window", &editor->showDemoWindow);

			ImGui::Separator();

			if (sceneManager)
			{
				std::vector<std::string> sceneIds;
				sceneManager->GetRegisteredSceneIds(sceneIds);

				const std::string currentSceneId = sceneManager->GetCurrentSceneId();
				const char* previewSceneName =
					currentSceneId.empty() ? "<No Scene Loaded>" : currentSceneId.c_str();

				ImGui::Text("Scene");
				if (ImGui::BeginCombo("Active Scene", previewSceneName))
				{
					for (const std::string& sceneId : sceneIds)
					{
						const bool isSelected = (sceneId == currentSceneId);

						if (ImGui::Selectable(sceneId.c_str(), isSelected))
						{
							if (sceneId != currentSceneId)
							{
								requestedSceneId = sceneId;
							}
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}

				if (ImGui::Button("Reload Scene"))
				{
					requestReloadScene = true;
				}

				ImGui::SameLine();
				ImGui::TextDisabled("%s", previewSceneName);
			}
			else
			{
				ImGui::TextDisabled("SceneManager not found.");
			}

			ImGui::Separator();

			if (editor->hasSelection)
			{
				ImGui::Text("Selected Entity: %u", static_cast<EZ::u32>(editor->selectedEntity));

				if (ImGui::Button("Clear Selection"))
				{
					editor->ClearSelection();
				}
			}
			else
			{
				ImGui::Text("Selected Entity: None");
			}
		}
		ImGui::End();

		if (sceneManager)
		{
			if (!requestedSceneId.empty())
			{
				editor->ClearSelection();
				sceneManager->LoadScene(requestedSceneId, world);
			}
			else if (requestReloadScene)
			{
				editor->ClearSelection();
				sceneManager->ReloadCurrentScene(world);
			}
		}

		if (editor->showWorldHierarchy && hierarchyPanel)
		{
			hierarchyPanel->Draw(world);
		}

		if (editor->showInspector && inspectorPanel)
		{
			inspectorPanel->Draw(world);
		}

		if (editor->showDemoWindow)
		{
			ImGui::ShowDemoWindow(&editor->showDemoWindow);
		}

		DrawFpsOverlay(project,world);

		ImGuiIO& io = ImGui::GetIO();
		editor->wantsCaptureMouse = io.WantCaptureMouse;
		editor->wantsCaptureKeyboard = io.WantCaptureKeyboard;

		imguiLayer->Render();
		device.EndPass();
	}
}