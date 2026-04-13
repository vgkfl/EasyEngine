#pragma once
#ifndef __TOOL_WORLD_HIERARCHY_PANEL_H__
#define __TOOL_WORLD_HIERARCHY_PANEL_H__

#include "core/Context/RunTimeContext/WorldContext.h"
#include "core/Entity/Entity.h"

namespace Tool
{
	class WorldHierarchyPanel
	{
	public:
		void Draw(EZ::WorldContext& world);

	private:
		void DrawEntityNode(EZ::WorldContext& world, EZ::Entity entity);
	};
}

#endif