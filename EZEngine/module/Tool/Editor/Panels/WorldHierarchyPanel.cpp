#include "Tool/Editor/Panels/WorldHierarchyPanel.h"

#include <optional>
#include <vector>

#include "imgui/imgui.h"

#include "ControlProtocol/EntityManager/EntityManager.h"
#include "ControlProtocol/TransformManager/TransformManager.h"
#include "Tool/Editor/EditorContext.h"

namespace
{
	static ImGuiTreeNodeFlags BuildNodeFlags(bool selected, bool hasChildren)
	{
		ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_OpenOnDoubleClick |
			ImGuiTreeNodeFlags_SpanAvailWidth;

		if (selected)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		if (!hasChildren)
		{
			flags |= ImGuiTreeNodeFlags_Leaf;
			flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		return flags;
	}
}

namespace Tool
{
	void WorldHierarchyPanel::Draw(EZ::WorldContext& world)
	{
		auto* editor = world.TryGet<Tool::EditorContext>();
		auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
		auto* transformManager = world.TryGet<ControlProtocol::TransformManager>();

		if (!editor || !entityManager || !transformManager)
		{
			return;
		}

		if (!ImGui::Begin("World Hierarchy", &editor->showWorldHierarchy))
		{
			ImGui::End();
			return;
		}

		std::vector<EZ::Entity> roots;
		transformManager->GetRootEntities(roots);

		if (roots.empty())
		{
			ImGui::TextDisabled("No hierarchy roots.");
			ImGui::End();
			return;
		}

		for (EZ::Entity entity : roots)
		{
			if (!entityManager->IsValid(entity))
			{
				continue;
			}

			DrawEntityNode(world, entity);
		}

		ImGui::End();
	}

	void WorldHierarchyPanel::DrawEntityNode(EZ::WorldContext& world, EZ::Entity entity)
	{
		auto* editor = world.TryGet<Tool::EditorContext>();
		auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
		auto* transformManager = world.TryGet<ControlProtocol::TransformManager>();

		if (!editor || !entityManager || !transformManager)
		{
			return;
		}

		const bool selected =
			editor->hasSelection &&
			editor->selectedEntity == entity;

		const bool hasChildren = transformManager->GetFirstChild(entity).has_value();

		const ImGuiTreeNodeFlags flags = BuildNodeFlags(selected, hasChildren);

		const EZ::u32 entityID = static_cast<EZ::u32>(entity);
		const bool open = ImGui::TreeNodeEx(
			reinterpret_cast<void*>(static_cast<uintptr_t>(entityID)),
			flags,
			"Entity ",
			entityID);

		if (ImGui::IsItemClicked())
		{
			editor->Select(entity);
		}

		if (!open || !hasChildren)
		{
			return;
		}

		std::optional<EZ::Entity> child = transformManager->GetFirstChild(entity);
		while (child.has_value())
		{
			if (entityManager->IsValid(*child))
			{
				DrawEntityNode(world, *child);
			}

			child = transformManager->GetNextSibling(*child);
		}

		ImGui::TreePop();
	}
}