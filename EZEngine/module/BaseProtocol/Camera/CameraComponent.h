#pragma once
#ifndef __B_P_CAMERA_COMPONENT_H__
#define __B_P_CAMERA_COMPONENT_H__

#include "core/Types.h"
#include "DataProtocol/MathTypes.h"

namespace BaseProtocol
{
	enum class CameraProjectionType : EZ::u8
	{
		Perspective = 0,
		Orthographic
	};

	struct CameraComponent
	{
		CameraProjectionType projectionType = CameraProjectionType::Perspective;

		EZ::f32 nearClip = 0.1f;
		EZ::f32 farClip = 1000.0f;

		EZ::f32 fovYDegrees = 60.0f;
		EZ::f32 orthoSize = 10.0f;

		bool isMainCamera = true;
		bool clearColorEnabled = true;
		bool clearDepthEnabled = true;

		DataProtocol::Vec4 clearColor{ 0.1f, 0.1f, 0.1f, 1.0f };
	};
}

#endif