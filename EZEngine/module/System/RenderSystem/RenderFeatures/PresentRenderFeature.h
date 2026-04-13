#pragma once
#ifndef __PRESENT_RENDER_FEATURE_H__
#define __PRESENT_RENDER_FEATURE_H__

#include "core/Render/IRenderFeature.h"

class PresentRenderFeature final : public EZ::IRenderFeature
{
public:
	const char* GetName() const override { return "PresentRenderFeature"; }
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