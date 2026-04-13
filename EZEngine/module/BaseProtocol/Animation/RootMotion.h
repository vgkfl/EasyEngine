#pragma once
#ifndef __B_P_ROOT_MOTION_H__
#define __B_P_ROOT_MOTION_H__

#include "DataProtocol/MathTypes.h"

namespace BaseProtocol
{
	struct RootMotionDelta
	{
		DataProtocol::Vec3 deltaPosition{ 0.0f, 0.0f, 0.0f };
		DataProtocol::Quat deltaRotation{ 0.0f, 0.0f, 0.0f, 1.0f };

		void Reset()
		{
			deltaPosition = { 0.0f, 0.0f, 0.0f };
			deltaRotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		}
	};
}

#endif