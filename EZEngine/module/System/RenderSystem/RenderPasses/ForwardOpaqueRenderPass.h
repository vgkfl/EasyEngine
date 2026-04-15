#pragma once
#ifndef __FORWARD_OPAQUE_RENDER_PASS_H__
#define __FORWARD_OPAQUE_RENDER_PASS_H__

#include "core/Render/IRenderPass.h"

class ForwardOpaqueRenderPass final : public EZ::IRenderPass
{
public:
	const char* GetName() const override
	{
		return "ForwardOpaqueRenderPass";
	}

	EZ::RenderPassEvent GetPassEvent() const override
	{
		return EZ::RenderPassEvent::Opaque;
	}

	void Setup(ControlProtocol::RenderPassContext& ctx) override;
	void Execute(ControlProtocol::RenderPassContext& ctx) override;
};

#endif