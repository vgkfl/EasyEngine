#pragma once
#ifndef __FORWARD_OPAQUE_RENDER_FEATURE_H__
#define __FORWARD_OPAQUE_RENDER_FEATURE_H__

#include "core/Render/IRenderFeature.h"

class ForwardOpaqueRenderFeature final : public EZ::IRenderFeature
{
public:
	const char* GetName() const override { return "ForwardOpaqueRenderFeature"; }
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

#endif