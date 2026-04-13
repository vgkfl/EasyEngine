#pragma once
#ifndef __B_P_BONE_POSE_COMPONENT_H__
#define __B_P_BONE_POSE_COMPONENT_H__

#include <vector>

#include "core/Types.h"
#include "DataProtocol/MathTypes.h"

namespace BaseProtocol
{
	struct BoneLocalPose
	{
		DataProtocol::Vec3 position{ 0.0f, 0.0f, 0.0f };
		DataProtocol::Quat rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		DataProtocol::Vec3 scale{ 1.0f, 1.0f, 1.0f };
	};

	struct BonePoseComponent
	{
		// ”Î SkeletonAsset::bones ∂‘∆Î
		std::vector<BoneLocalPose> localPoses;
		bool dirty = true;
	};
}

#endif