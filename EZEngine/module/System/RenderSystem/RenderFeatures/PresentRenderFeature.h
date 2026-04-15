#pragma once
#ifndef __PRESENT_RENDER_FEATURE_H__
#define __PRESENT_RENDER_FEATURE_H__

#include "core/Render/IRenderFeature.h"
#include "DataProtocol/Render/RenderFeatureSettings.h"
#include "System/RenderSystem/RenderPasses/PresentRenderPass.h"

class PresentRenderFeature final : public EZ::IRenderFeature
{
public:
	const char* GetName() const override
	{
		return "PresentRenderFeature";
	}

	bool IsEnabled(const ControlProtocol::RenderPassContext& ctx) const override;
	void CollectPasses(
		ControlProtocol::RenderPassQueue& queue,
		ControlProtocol::RenderPassContext& ctx) override;

	DataProtocol::PresentFeatureSettings& Settings()
	{
		return m_Settings;
	}

	const DataProtocol::PresentFeatureSettings& Settings() const
	{
		return m_Settings;
	}

private:
	DataProtocol::PresentFeatureSettings m_Settings{};
	PresentRenderPass m_PresentPass;
};

#endif