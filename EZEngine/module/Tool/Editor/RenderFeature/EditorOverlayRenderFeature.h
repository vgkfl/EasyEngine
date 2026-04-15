#pragma once
#ifndef __TOOL_EDITOR_OVERLAY_RENDER_FEATURE_H__
#define __TOOL_EDITOR_OVERLAY_RENDER_FEATURE_H__

#include "core/Render/IRenderFeature.h"
#include "Tool/Editor/RenderPasses/CopyToFinalRenderPass.h"
#include "Tool/Editor/RenderPasses/EditorOverlayRenderPass.h"

namespace Tool
{
	class EditorOverlayRenderFeature final : public EZ::IRenderFeature
	{
	public:
		const char* GetName() const override
		{
			return "EditorOverlayRenderFeature";
		}

		bool IsEnabled(const ControlProtocol::RenderPassContext& ctx) const override;
		void CollectPasses(
			ControlProtocol::RenderPassQueue& queue,
			ControlProtocol::RenderPassContext& ctx) override;

	private:
		CopyToFinalRenderPass m_CopyToFinalPass;
		EditorOverlayRenderPass m_EditorOverlayPass;
	};
}

#endif