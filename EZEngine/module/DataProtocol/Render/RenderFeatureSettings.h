#pragma once
#ifndef __D_P_RENDER_FEATURE_SETTINGS_H__
#define __D_P_RENDER_FEATURE_SETTINGS_H__

#include "core/Types.h"
#include "core/Render/RenderPassEvent.h"

namespace DataProtocol
{
	struct ForwardOpaqueFeatureSettings
	{
		bool enabled = true;
		EZ::RenderPassEvent passEvent = EZ::RenderPassEvent::Opaque;
	};

	struct PresentFeatureSettings
	{
		bool enabled = true;
		EZ::RenderPassEvent passEvent = EZ::RenderPassEvent::Present;
	};

	struct PerObjectShadowFeatureSettings
	{
		bool enabled = false;
		EZ::RenderPassEvent passEvent = EZ::RenderPassEvent::BeforeOpaque;

		EZ::u32 shadowMapSize = 4096;
		float depthBias = 2.5f;
		float normalBias = 5.0f;

		bool enablePCSS = true;
		EZ::u32 poissonRingCount = 16;
		float lightSize = 0.024f;
	};

	struct DepthNormalFeatureSettings
	{
		bool enabled = false;
		EZ::RenderPassEvent passEvent = EZ::RenderPassEvent::Prepass;
	};

	struct StylizedLightingFeatureSettings
	{
		bool enabled = false;
		EZ::RenderPassEvent passEvent = EZ::RenderPassEvent::BeforeOpaque;

		float flattenSH = 1.0f;
		float environmentBlend = 0.0f;
	};
}

#endif