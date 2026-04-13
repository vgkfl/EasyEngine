#pragma once
#ifndef __TOOL_INSPECTOR_PANEL_H__
#define __TOOL_INSPECTOR_PANEL_H__

#include "core/Context/RunTimeContext/WorldContext.h"

namespace Tool
{
	class InspectorPanel
	{
	public:
		void Draw(EZ::WorldContext& world);
	};
}

#endif