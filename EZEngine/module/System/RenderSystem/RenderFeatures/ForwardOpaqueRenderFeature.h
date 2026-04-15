#pragma once
#ifndef __FORWARD_OPAQUE_RENDER_FEATURE_H__
#define __FORWARD_OPAQUE_RENDER_FEATURE_H__

#include "core/Render/IRenderFeature.h"
#include "DataProtocol/Render/RenderFeatureSettings.h"
#include "System/RenderSystem/RenderPasses/ForwardOpaqueRenderPass.h"

class ForwardOpaqueRenderFeature final : public EZ::IRenderFeature
{
public:
	const char* GetName() const override
	{
		return "ForwardOpaqueRenderFeature";
	}

	bool IsEnabled(const ControlProtocol::RenderPassContext& ctx) const override;
	void CollectPasses(
		ControlProtocol::RenderPassQueue& queue,
		ControlProtocol::RenderPassContext& ctx) override;

	DataProtocol::ForwardOpaqueFeatureSettings& Settings()
	{
		return m_Settings;
	}

	const DataProtocol::ForwardOpaqueFeatureSettings& Settings() const
	{
		return m_Settings;
	}

private:
	DataProtocol::ForwardOpaqueFeatureSettings m_Settings{};
	ForwardOpaqueRenderPass m_ForwardOpaquePass;
};

#endif