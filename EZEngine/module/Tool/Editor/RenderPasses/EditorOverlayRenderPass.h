#pragma once
#ifndef __TOOL_EDITOR_OVERLAY_RENDER_PASS_H__
#define __TOOL_EDITOR_OVERLAY_RENDER_PASS_H__

#include "core/Render/IRenderPass.h"

namespace Tool
{
	class EditorOverlayRenderPass final : public EZ::IRenderPass
	{
	public:
		const char* GetName() const override
		{
			return "EditorOverlayRenderPass";
		}

		EZ::RenderPassEvent GetPassEvent() const override
		{
			return EZ::RenderPassEvent::Overlay;
		}

		void Setup(ControlProtocol::RenderPassContext& ctx) override;
		void Execute(ControlProtocol::RenderPassContext& ctx) override;
	};
}

#endif