#pragma once
#ifndef __TOOL_EDITOR_OVERLAY_RENDER_FEATURE_H__
#define __TOOL_EDITOR_OVERLAY_RENDER_FEATURE_H__

#include "core/Render/IRenderFeature.h"

namespace Tool
{
	class EditorOverlayRenderFeature final : public EZ::IRenderFeature
	{
	public:
		const char* GetName() const override { return "EditorOverlayRenderFeature"; }
		bool IsEnabled() const override { return true; }

		void Setup(
			EZ::ProjectContext& project,
			EZ::WorldContext& world,
			ControlProtocol::RenderImageBuffer& imageBuffer,
			ControlProtocol::RenderDeviceController& device) override;

		void Execute(
			EZ::ProjectContext& project,
			EZ::WorldContext& world,
			ControlProtocol::RenderImageBuffer& imageBuffer,
			ControlProtocol::RenderDeviceController& device) override;
	};
}

#endif