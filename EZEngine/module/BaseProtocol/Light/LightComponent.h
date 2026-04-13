#pragma once
#ifndef __B_P_LIGHT_COMPONENT_H__
#define __B_P_LIGHT_COMPONENT_H__

#include "core/Types.h"
#include "DataProtocol/MathTypes.h"

namespace BaseProtocol
{
	enum class LightType : EZ::u8
	{
		Directional = 0,
		Point,
		Spot
	};

	struct LightComponent
	{
		LightType type = LightType::Directional;

		DataProtocol::Vec3 color{ 1.0f, 1.0f, 1.0f };
		EZ::f32 intensity = 1.0f;

		EZ::f32 range = 10.0f;
		EZ::f32 spotAngleDegrees = 45.0f;

		bool castShadow = false;

		EZ::f32 depthBias = 1.0f;
		EZ::f32 normalBias = 1.0f;
		EZ::f32 slopeScaleDepthBias = 1.0f;
	};
}

#endif