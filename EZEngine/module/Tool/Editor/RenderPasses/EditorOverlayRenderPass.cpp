#include "Tool/Editor/RenderPasses/EditorOverlayRenderPass.h"

#include "imgui/imgui.h"

#include "ControlProtocol/RenderPipelineController/RenderPassQueue.h"
#include "Tool/Editor/EditorContext.h"
#include "Tool/Editor/Imgui/ImGuiLayer.h"
#include "Tool/Editor/Panels/InspectorPanel.h"
#include "Tool/Editor/Panels/PerformancePanel.h"
#include "Tool/Editor/Panels/SceneControlPanel.h"
#include "Tool/Editor/Panels/WorldHierarchyPanel.h"

namespace
{
	void DrawEditorRootWindow(EZ::WorldContext& world)
	{
		auto* editor = world.TryGet<Tool::EditorContext>();
		if (!editor)
		{
			return;
		}

		if (!ImGui::Begin("Editor"))
		{
			ImGui::End();
			return;
		}

		ImGui::Checkbox("World Hierarchy", &editor->showWorldHierarchy);
		ImGui::Checkbox("Inspector", &editor->showInspector);
		ImGui::Checkbox("Scene Control", &editor->showSceneControlPanel);
		ImGui::Checkbox("Performance Monitor", &editor->showPerformancePanel);
		ImGui::Checkbox("Demo Window", &editor->showDemoWindow);

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

		ImGui::End();
	}
}

namespace Tool
{
	void EditorOverlayRenderPass::Setup(ControlProtocol::RenderPassContext& ctx)
	{
		(void)ctx;
	}

	void EditorOverlayRenderPass::Execute(ControlProtocol::RenderPassContext& ctx)
	{
		auto* editor = ctx.world.TryGet<Tool::EditorContext>();
		auto* imguiLayer = ctx.world.TryGet<Tool::ImGuiLayer>();
		auto* hierarchyPanel = ctx.world.TryGet<Tool::WorldHierarchyPanel>();
		auto* inspectorPanel = ctx.world.TryGet<Tool::InspectorPanel>();
		auto* sceneControlPanel = ctx.world.TryGet<Tool::SceneControlPanel>();
		auto* performancePanel = ctx.world.TryGet<Tool::PerformancePanel>();

		if (!editor || !editor->enabled || !imguiLayer || !imguiLayer->IsInitialized())
		{
			return;
		}

		auto* dst = ctx.imageBuffer.TryGet(EZ::RenderImageTag::FinalColor);
		if (!dst)
		{
			return;
		}

		if (!ctx.device.BeginPass(dst, nullptr))
		{
			return;
		}

		imguiLayer->BeginFrame();

		DrawEditorRootWindow(ctx.world);

		if (editor->showWorldHierarchy && hierarchyPanel)
		{
			hierarchyPanel->Draw(ctx.world);
		}

		if (editor->showInspector && inspectorPanel)
		{
			inspectorPanel->Draw(ctx.world);
		}

		if (sceneControlPanel)
		{
			sceneControlPanel->Draw(ctx.world);
		}

		if (performancePanel)
		{
			performancePanel->Draw(ctx.project, ctx.world);
		}

		if (editor->showDemoWindow)
		{
			ImGui::ShowDemoWindow(&editor->showDemoWindow);
		}

		ImGuiIO& io = ImGui::GetIO();
		editor->wantsCaptureMouse = io.WantCaptureMouse;
		editor->wantsCaptureKeyboard = io.WantCaptureKeyboard;

		imguiLayer->Render();
		ctx.device.EndPass();

		// 场景切换放到本帧 UI 全部绘制完成之后再执行，避免中途改世界状态。
		if (sceneControlPanel)
		{
			sceneControlPanel->ApplyPending(ctx.world);
		}
	}
}