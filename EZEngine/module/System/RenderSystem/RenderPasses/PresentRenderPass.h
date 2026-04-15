#pragma once
#ifndef __PRESENT_RENDER_PASS_H__
#define __PRESENT_RENDER_PASS_H__

#include "core/Render/IRenderPass.h"

class PresentRenderPass final : public EZ::IRenderPass
{
public:
	const char* GetName() const override
	{
		return "PresentRenderPass";
	}

	EZ::RenderPassEvent GetPassEvent() const override
	{
		return EZ::RenderPassEvent::Present;
	}

	void Setup(ControlProtocol::RenderPassContext& ctx) override;
	void Execute(ControlProtocol::RenderPassContext& ctx) override;
};

#endif