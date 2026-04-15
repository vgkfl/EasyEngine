#pragma once
#ifndef __TOOL_EDITOR_CONTEXT_H__
#define __TOOL_EDITOR_CONTEXT_H__

#include "core/Entity/Entity.h"
#include "core/Types.h"

namespace Tool
{
	enum class GizmoOperation : EZ::u8
	{
		None = 0,
		Translate,
		Rotate,
		Scale
	};

	enum class GizmoSpace : EZ::u8
	{
		Local = 0,
		World
	};

	struct EditorContext
	{
		bool enabled = true;

		EZ::Entity selectedEntity = EZ::Entity{};
		bool hasSelection = false;

		GizmoOperation gizmoOperation = GizmoOperation::Translate;
		GizmoSpace gizmoSpace = GizmoSpace::Local;

		bool wantsCaptureMouse = false;
		bool wantsCaptureKeyboard = false;

		bool gizmoHovered = false;
		bool gizmoActive = false;

		bool showWorldHierarchy = true;
		bool showInspector = true;
		bool showSceneControlPanel = true;
		bool showPerformancePanel = true;
		bool showDemoWindow = false;

		void ClearSelection()
		{
			selectedEntity = EZ::Entity{};
			hasSelection = false;
		}

		void Select(EZ::Entity entity)
		{
			selectedEntity = entity;
			hasSelection = true;
		}
	};
}

#endif