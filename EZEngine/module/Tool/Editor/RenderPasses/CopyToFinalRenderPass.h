#pragma once
#ifndef __TOOL_COPY_TO_FINAL_RENDER_PASS_H__
#define __TOOL_COPY_TO_FINAL_RENDER_PASS_H__

#include "core/Render/IRenderPass.h"

namespace Tool
{
	class CopyToFinalRenderPass final : public EZ::IRenderPass
	{
	public:
		const char* GetName() const override
		{
			return "CopyToFinalRenderPass";
		}

		EZ::RenderPassEvent GetPassEvent() const override
		{
			return EZ::RenderPassEvent::AfterOpaque;
		}

		EZ::u16 GetPassOrder() const override
		{
			return 100;
		}

		void Setup(ControlProtocol::RenderPassContext& ctx) override;
		void Execute(ControlProtocol::RenderPassContext& ctx) override;
	};
}

#endif