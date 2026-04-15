#pragma once
#ifndef __TOOL_SCENE_CONTROL_PANEL_H__
#define __TOOL_SCENE_CONTROL_PANEL_H__

#include <string>

#include "core/Context/RunTimeContext/WorldContext.h"

namespace Tool
{
	class SceneControlPanel
	{
	public:
		void Draw(EZ::WorldContext& world);
		void ApplyPending(EZ::WorldContext& world);

	private:
		std::string m_SelectedSceneId;
		std::string m_PendingLoadSceneId;
		bool m_RequestReloadScene = false;
	};
}

#endif