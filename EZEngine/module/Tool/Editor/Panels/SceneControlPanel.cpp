#include "Tool/Editor/Panels/SceneControlPanel.h"

#include <algorithm>
#include <vector>

#include "imgui/imgui.h"

#include "Launcher/GameLauncher/Scene/SceneManager.h"
#include "Tool/Editor/EditorContext.h"

namespace
{
	static bool ContainsSceneId(
		const std::vector<std::string>& sceneIds,
		const std::string& sceneId)
	{
		return std::find(sceneIds.begin(), sceneIds.end(), sceneId) != sceneIds.end();
	}
}

namespace Tool
{
	void SceneControlPanel::Draw(EZ::WorldContext& world)
	{
		auto* editor = world.TryGet<Tool::EditorContext>();
		auto* sceneManager = world.TryGet<GameScene::SceneManager>();

		if (!editor || !editor->enabled || !editor->showSceneControlPanel)
		{
			return;
		}

		if (!ImGui::Begin("Scene Control", &editor->showSceneControlPanel))
		{
			ImGui::End();
			return;
		}

		if (!sceneManager)
		{
			ImGui::TextDisabled("SceneManager not found.");
			ImGui::End();
			return;
		}

		std::vector<std::string> sceneIds;
		sceneManager->GetRegisteredSceneIds(sceneIds);

		if (sceneIds.empty())
		{
			ImGui::TextDisabled("No registered scenes.");
			ImGui::End();
			return;
		}

		const std::string currentSceneId = sceneManager->GetCurrentSceneId();

		if (!ContainsSceneId(sceneIds, m_SelectedSceneId))
		{
			if (!currentSceneId.empty() && ContainsSceneId(sceneIds, currentSceneId))
			{
				m_SelectedSceneId = currentSceneId;
			}
			else
			{
				m_SelectedSceneId = sceneIds.front();
			}
		}

		ImGui::Text("Current Scene");
		ImGui::SameLine();
		ImGui::TextDisabled("%s", currentSceneId.empty() ? "<No Scene Loaded>" : currentSceneId.c_str());

		ImGui::Spacing();

		const char* previewName =
			m_SelectedSceneId.empty() ? "<No Scene Selected>" : m_SelectedSceneId.c_str();

		if (ImGui::BeginCombo("Target Scene", previewName))
		{
			for (const std::string& sceneId : sceneIds)
			{
				const bool selected = (sceneId == m_SelectedSceneId);

				if (ImGui::Selectable(sceneId.c_str(), selected))
				{
					m_SelectedSceneId = sceneId;
				}

				if (selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		const bool canLoad =
			!m_SelectedSceneId.empty() &&
			m_SelectedSceneId != currentSceneId;

		if (!canLoad)
		{
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Load Selected Scene"))
		{
			m_PendingLoadSceneId = m_SelectedSceneId;
		}

		if (!canLoad)
		{
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		if (!sceneManager->HasCurrentScene())
		{
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Reload Current Scene"))
		{
			m_RequestReloadScene = true;
		}

		if (!sceneManager->HasCurrentScene())
		{
			ImGui::EndDisabled();
		}

		ImGui::End();
	}

	void SceneControlPanel::ApplyPending(EZ::WorldContext& world)
	{
		auto* editor = world.TryGet<Tool::EditorContext>();
		auto* sceneManager = world.TryGet<GameScene::SceneManager>();

		if (!editor || !sceneManager)
		{
			m_PendingLoadSceneId.clear();
			m_RequestReloadScene = false;
			return;
		}

		if (!m_PendingLoadSceneId.empty())
		{
			editor->ClearSelection();
			sceneManager->LoadScene(m_PendingLoadSceneId, world);
			m_SelectedSceneId = sceneManager->GetCurrentSceneId();
			m_PendingLoadSceneId.clear();
			m_RequestReloadScene = false;
			return;
		}

		if (m_RequestReloadScene)
		{
			editor->ClearSelection();
			sceneManager->ReloadCurrentScene(world);
			m_SelectedSceneId = sceneManager->GetCurrentSceneId();
			m_RequestReloadScene = false;
		}
	}
}